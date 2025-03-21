// src/netfilter/NetfilterQueue.cpp

#include "NetfilterQueue.hpp"
#include "Packet.hpp"
#include "configs.hpp"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>

#include <libnetfilter_queue/libnetfilter_queue.h>
#include <libnfnetlink/linux_nfnetlink.h>

#include <netinet/in.h>
#include <random>
#include <stdexcept>
#include <thread>

int transmission_delay = 1.38;
float transmission_min = 1.3;

NetfilterQueue::NetfilterQueue()
    : running_(true), handle_(nullptr, nfq_close),
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
}

void NetfilterQueue::stop() { running_ = false; }

bool NetfilterQueue::isRunning() const { return running_; }

int NetfilterQueue::packetCallbackStatic(struct nfq_q_handle *qh,
                                         struct nfgenmsg *nfmsg,
                                         struct nfq_data *nfa, void *data) {
  // cast the void pointer back to the class instance
  return static_cast<NetfilterQueue *>(data)->packetCallback(qh, nfmsg, nfa);
}

// Flip a random bit due to solar flares
static void mario_cart_speedrun_simulation(Packet &packet) {
  uint8_t *data = packet.getMutableData();
  size_t len = packet.getLength();
  if (!data || len == 0) {
    return;
  }
  thread_local std::mt19937 engine(std::random_device{}());

  std::uniform_int_distribution<int> do_a_mario_cart_speedrun(0, 100000);

  if (do_a_mario_cart_speedrun(engine) == 0) {
    std::cout << "It's a me, Mario!\n";
    std::uniform_int_distribution<size_t> byteDist(0, len - 1);
    std::uniform_int_distribution<int> bitPosDist(0, 7);
    size_t bytePos = byteDist(engine);
    int bitPos = bitPosDist(engine);
    data[bytePos] ^= (1 << bitPos);
  }
}

// Generate a base delay in seconds
static double generateBaseDelay() {
  thread_local std::mt19937 engine(std::random_device{}());
  std::poisson_distribution<int> distribution(transmission_delay);
  double delay = distribution(engine);
  if (delay < transmission_min) {
    delay = transmission_min;
  }
  return delay;
}

// Helper function: Corrupt a small block of the packet's payload
static bool corrupt_packet(Packet &packet) {
  uint8_t *data = packet.getMutableData();
  size_t len = packet.getLength();
  constexpr size_t block_size = 64;
  bool error_occurred = false;

  // Define the six states.
  constexpr int NUM_STATES = 6;

  // Transition matrix: rows are current state, columns are next state.
  double transition_matrix[NUM_STATES][NUM_STATES] = {
      {0.95, 0.04, 0.005, 0.005, 0.0, 0.0}, // from state 0 (no corruption)
      {0.10, 0.80, 0.08, 0.02, 0.0, 0.0},   // from state 1 (very good)
      {0.05, 0.10, 0.70, 0.10, 0.05, 0.0},  // from state 2 (good)
      {0.02, 0.05, 0.15, 0.60, 0.15, 0.03}, // from state 3 (regular)
      {0.01, 0.02, 0.07, 0.20, 0.60, 0.10}, // from state 4 (bad)
      {0.05, 0.05, 0.10, 0.20, 0.30, 0.30}  // from state 5 (very bad)
  };

  // Corruption parameters for states 0-5: (mean, std dev)
  struct CorruptionParam {
    double mean;
    double std;
  };
  CorruptionParam corruption_params[NUM_STATES] = {
      {0, 0},   // state 0: no corruption
      {1, 0.5}, // state 1: very good (minimal corruption)
      {2, 1},   // state 2: good (low-level corruption)
      {3, 2},   // state 3: regular (moderate corruption)
      {5, 3},   // state 4: bad (severe corruption)
      {12, 3}   // state 5: very bad (extensive corruption)
  };

  // Setup random number generators.
  std::random_device rd;
  std::mt19937 rng(rd());
  std::uniform_int_distribution<int> byte_dist(0, 255);

  int current_state = 0; // Start in state 0 (no corruption)

  // Process the packet block by block.
  for (size_t block_start = 0; block_start < len; block_start += block_size) {
    size_t block_end = std::min(block_start + block_size, len);
    size_t block_length = block_end - block_start;

    // Only corrupt if current state is non-zero.
    if (current_state != 0) {
      CorruptionParam param = corruption_params[current_state];
      // Sample the number of bytes to corrupt using a Gaussian distribution.
      std::normal_distribution<double> norm_dist(param.mean, param.std);
      int corruption_size = static_cast<int>(std::round(norm_dist(rng)));
      // Clamp corruption size to be at least 1 and at most the block length.
      int block_corrupt_size = std::max(
          1, std::min(corruption_size, static_cast<int>(block_length)));

      // Choose a random starting index within the block.
      std::uniform_int_distribution<size_t> start_index_dist(
          0, block_length - block_corrupt_size);
      size_t start_index = start_index_dist(rng);

      // Overwrite the chosen segment with random bytes.
      for (size_t i = start_index; i < start_index + block_corrupt_size; ++i) {
        data[block_start + i] = static_cast<uint8_t>(byte_dist(rng));
      }
      error_occurred = true;
    }

    // Update the state using the transition matrix.
    std::uniform_real_distribution<double> real_dist(0.0, 1.0);
    double p = real_dist(rng);
    double cumulative = 0.0;
    int next_state = 0;
    for (int state = 0; state < NUM_STATES; ++state) {
      cumulative += transition_matrix[current_state][state];
      if (p < cumulative) {
        next_state = state;
        break;
      }
    }
    current_state = next_state;
  }

  return error_occurred;
}

// The simulation function that applies delay, and bit corruption.
bool simulate_moon_earth_channel(Packet &packet) {

  // Drop the packed if vibes are bad
  thread_local std::mt19937 engine(std::random_device{}());
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  if (dist(engine) < 0.05) {
    packet.getMutableData()[0] = 0x00;
    return true;
  }

  // 1. Calculate total delay: base delay plus jitter.
  double Delay = generateBaseDelay(); // in seconds

  // 2. Apply delay (blocking call). (Note: this will block the current thread,
  // which should not be an issue in this case since we are running NFQUEUE
  std::this_thread::sleep_for(std::chrono::duration<double>(Delay));

  corrupt_packet(packet);
  mario_cart_speedrun_simulation(packet);

  return true; // Indicate simulation was successful.
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

    auto now = std::chrono::steady_clock::now();
    Packet packet(id, packet_data, payload_len, mark, now, false);

    ///////////////////////////////////////////////////////////////////

    // PACKET PROCESSING LOGIC
    // Just printing classification for now

    std::cout << "Packet received!\n";
    std::cout << "ID: " << id;
    std::cout << "\nClassification: " << packet.getLinkTypeName();
    std::cout << "\nSize: " << payload_len << " bytes\n";

    bool simulation_successfull = simulate_moon_earth_channel(packet);

    if (!simulation_successfull) {
      std::cout << "Simulation failed!\n";
    }

    ///////////////////////////////////////////////////////////////////

    // currently just accepting everything but that will change later
    return nfq_set_verdict(qh, id, NF_ACCEPT, payload_len, packet_data);
  } catch (std::exception &error) {
    std::cerr << "Error processing packet: " << error.what() << "\n";
    return nfq_set_verdict(qh, id, NF_ACCEPT, 0, nullptr);
  }
}