// src/netfilter/NetfilterQueue.cpp

#include <cassert>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <exception>
#include <iostream>
#include <stdexcept>

#include <libnetfilter_queue/libnetfilter_queue.h>
#include <libnfnetlink/linux_nfnetlink.h>
#include <netinet/in.h>

#include "NetfilterQueue.hpp"
#include <random>

NetfilterQueue::NetfilterQueue(ConfigManager &config_manager)
    : config_manager_(config_manager), running_(true),
      handle_(nullptr, nfq_close),
      queue_handle_(nullptr, [](struct nfq_q_handle *qh) {
        if (qh) {
          std::cout << "Destroying queue.\n";
          nfq_destroy_queue(qh);
        }
      }) {

  std::cout << "Opening Netfilter queue.\n";

  // Open queue handle
  struct nfq_handle *h = nfq_open();
  if (!h) {
    throw std::runtime_error("Failed to open netfilter queue");
  }
  handle_.reset(h);

  // Unbind and rebind IPv4
  if (nfq_unbind_pf(handle_.get(), AF_INET) < 0) {
    throw std::runtime_error("Failed to unbind IPv4 from netfilter queue");
  }

  if (nfq_bind_pf(handle_.get(), AF_INET) < 0) {
    throw std::runtime_error("Failed to bind IPv4 to netfilter queue");
  }

  std::cout << "Creating queue and setting callback..." << "\n";

  // Create the queue with the callback function
  struct nfq_q_handle *qh = nfq_create_queue(
      handle_.get(), QUEUE_NUM, &NetfilterQueue::packetCallbackStatic, this);
  if (!qh) {
    throw std::runtime_error("Failed to create netfilter queue");
  }

  // Set the queue handle
  queue_handle_.reset(qh);

  // Set copy packet mode
  if (nfq_set_mode(queue_handle_.get(), NFQNL_COPY_PACKET, MAX_PACKET_SIZE) <
      0) {
    throw std::runtime_error("Failed to set netfilter queue copy mode");
  }

  // Get the socket file descriptor
  fd_ = nfq_fd(handle_.get());

  // Increase socket buffer size
  int opt = SOCKET_BUFFER_SIZE;
  if (setsockopt(fd_, SOL_SOCKET, SO_RCVBUF, &opt, sizeof(opt)) < 0) {
    std::cerr << "Warning: Could not increase socket buffer size.\n";
  }
}

void NetfilterQueue::run() {
  std::cout << "Starting main packet processing loop.\n";

  // Start burst error simulation threads
  moon_to_earth_burst_thread_ =
      std::thread(&NetfilterQueue::burstErrorSimulation, this,
                  Packet::LinkType::MOON_TO_EARTH);
  earth_to_moon_burst_thread_ =
      std::thread(&NetfilterQueue::burstErrorSimulation, this,
                  Packet::LinkType::EARTH_TO_MOON);
  moon_to_moon_burst_thread_ =
      std::thread(&NetfilterQueue::burstErrorSimulation, this,
                  Packet::LinkType::MOON_TO_MOON);

  char buffer[MAX_PACKET_SIZE] __attribute__((aligned));

  while (running_) {
    // receive data from the netfilter queue
    ssize_t received = recv(fd_, (char *)buffer, sizeof(buffer), 0);

    if (received >= 0) {
      nfq_handle_packet(handle_.get(), (char *)buffer,
                        static_cast<int>(received));
      continue;
    }

    // Handle buffer overflowing
    if (errno == ENOBUFS) {
      std::cerr << "Warning: Buffer overflows, packets are being dropped!\n";
      continue;
    }

    if (errno == EINTR) {
      // interrupted by signal, check if we should continue
      if (!running_) {
        break;
      }
      continue;
    }

    // any other error
    throw std::runtime_error("recv() failed");
  }

  std::cout << "Exiting main packet processing loop.\n";

  // Join threads
  if (moon_to_earth_burst_thread_.joinable()) {
    moon_to_earth_burst_thread_.join();
  }

  if (earth_to_moon_burst_thread_.joinable()) {
    earth_to_moon_burst_thread_.join();
  }

  if (moon_to_moon_burst_thread_.joinable()) {
    moon_to_moon_burst_thread_.join();
  }

  std::cout << "All burst simulation threads terminated.\n";
}

void NetfilterQueue::stop() {
  burst_threads_running_ = false;
  running_ = false;
}

bool NetfilterQueue::isRunning() const { return running_; }

int NetfilterQueue::packetCallbackStatic(struct nfq_q_handle *qh,
                                         struct nfgenmsg *nfmsg,
                                         struct nfq_data *nfa, void *data) {
  // cast the void pointer back to the class instance
  return static_cast<NetfilterQueue *>(data)->packetCallback(qh, nfmsg, nfa);
}

int NetfilterQueue::packetCallback(struct nfq_q_handle *qh,
                                   struct nfgenmsg *nfmsg,
                                   struct nfq_data *nfa) {

  // get the packet header
  struct nfqnl_msg_packet_hdr *ph = nfq_get_msg_packet_hdr(nfa);
  uint32_t id = 0;

  // get packet id
  if (ph) {
    id = ntohl(ph->packet_id);
  } else {
    std::cerr << "Warning: Couldn't get packet header.\n";
    return nfq_set_verdict(qh, id, NF_ACCEPT, 0, nullptr);
  }

  // get packet mark
  uint32_t mark = nfq_get_nfmark(nfa);

  // get packet payload
  uint8_t *packet_data = nullptr;
  int payload_len = nfq_get_payload(nfa, &packet_data);

  if (payload_len < 0) {
    std::cerr << "Error: Couldm't get packet payload.\n";
    return nfq_set_verdict(qh, id, NF_ACCEPT, 0, nullptr);
  }

  try {
    // Process the packet
    // This is where the magic will theoretically happen
    // If this code ever sees the light of day, that is

    auto now = std::chrono::steady_clock::now();
    Packet packet(id, packet_data, payload_len, mark, now, false);

    ///////////////////////////////////////////////////////////////////

    // PACKET PROCESSING LOGIC

    // Apply mark and burst error_condition based on link type
    uint32_t new_mark = 0;
    bool is_in_burst_error = false;
    switch (packet.getLinkType()) {
    case Packet::LinkType::EARTH_TO_EARTH:
      new_mark = MARK_EARTH_TO_EARTH;
      break;
    case Packet::LinkType::EARTH_TO_MOON:
      new_mark = MARK_EARTH_TO_MOON;
      is_in_burst_error = burst_error_earth_to_moon_;
      break;
    case Packet::LinkType::MOON_TO_EARTH:
      new_mark = MARK_MOON_TO_EARTH;
      is_in_burst_error = burst_error_moon_to_earth_;
      break;
    case Packet::LinkType::MOON_TO_MOON:
      new_mark = MARK_MOON_TO_MOON;
      is_in_burst_error = burst_error_moon_to_moon_;
      break;
    default:
      new_mark = 0; // Unclassified traffic gets no mark
      break;
    }

    // Some debugging output for now
    std::cout << "Packet received!\n";
    std::cout << "ID: " << id;
    std::cout << "\nClassification: " << packet.getLinkTypeName();
    std::cout << "\nSize: " << payload_len << " bytes";
    std::cout << "\nApplying mark: " << new_mark << "\n";

    ///////////////////////////////////////////////////////////////////

    // If the link types' burst mode is enabled, drop the packet
    if (is_in_burst_error) {
      return nfq_set_verdict2(qh, id, NF_DROP, new_mark, packet.getLength(),
                              packet.getData());
    }

    // using packet.getData() in case the packet was modified
    return nfq_set_verdict2(qh, id, NF_ACCEPT, new_mark, packet.getLength(),
                            packet.getData());
  } catch (std::exception &error) {
    std::cerr << "Error processing packet: " << error.what() << "\n";
    return nfq_set_verdict2(qh, id, NF_ACCEPT, MARK_EARTH_TO_EARTH, 0, nullptr);
  }
}

void NetfilterQueue::burstErrorSimulation(const Packet::LinkType link_type) {

  // Get the correct burst_error atomic variable based on the link type
  std::atomic<bool> &burst_error_mode = [this,
                                         link_type]() -> std::atomic<bool> & {
    switch (link_type) {
    case Packet::LinkType::EARTH_TO_MOON:
      return burst_error_earth_to_moon_;
    case Packet::LinkType::MOON_TO_EARTH:
      return burst_error_moon_to_earth_;
    case Packet::LinkType::MOON_TO_MOON:
      return burst_error_moon_to_moon_;
    default:
      // This should never happen, but just in case
      throw std::invalid_argument(
          "Invalid link type for burst error simulation.");
    }
  }();

  // Get the link properties for the current link type
  Config::LinkProperties props = [this, link_type]() -> Config::LinkProperties {
    switch (link_type) {
    case Packet::LinkType::EARTH_TO_MOON:
      return config_manager_.getEToMConfig();
    case Packet::LinkType::MOON_TO_EARTH:
      return config_manager_.getMToEConfig();
    case Packet::LinkType::MOON_TO_MOON:
      return config_manager_.getMToMConfig();
    default:
      // This also should never happen, but just in case
      throw std::invalid_argument(
          "Invalid link type for burst error simulation.");
    }
  }();

  // Random number generator for burst error simulation
  std::mt19937 random_generator(time(0));

  // Initialize the burst error mode
  burst_error_mode = false;

  std::normal_distribution<double> freq_dist(
      props.base_packet_loss_burst_freq_per_minute,
      props.packet_loss_burst_freq_stddev);

  std::normal_distribution<double> burst_dist(
      props.base_packet_loss_burst_duration_ms,
      props.base_packet_loss_burst_duration_stddev);

  // Define the burst error simulation parameters
  uint64_t ms_to_next_burst, ms_burst_duration;

  while (burst_threads_running_) {
    // Generate random values for the frequency and duration
    ms_to_next_burst =
        static_cast<uint64_t>(6000.0 / freq_dist(random_generator));
    ms_burst_duration = static_cast<uint64_t>(burst_dist(random_generator));

    // Sleep for the next burst duration
    std::this_thread::sleep_for(std::chrono::milliseconds(ms_to_next_burst));

    // Check if the thread should stop
    if (!burst_threads_running_)
      break;

    // Enable burst error mode
    burst_error_mode = true;

    // Sleep for the burst duration
    std::this_thread::sleep_for(std::chrono::milliseconds(ms_burst_duration));

    burst_error_mode = false;
  }

  // Reset the burst error mode
  burst_error_mode = false;
}