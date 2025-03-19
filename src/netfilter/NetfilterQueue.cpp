// src/netfilter/NetfilterQueue.cpp

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
#include "Packet.hpp"
#include "configs.hpp"

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
    // Just printing classification for now

    std::cout << "Packet received!\n";
    std::cout << "ID: " << id;
    std::cout << "\nClassification: " << packet.getLinkTypeName();
    std::cout << "\nSize: " << payload_len << " bytes\n";

    ///////////////////////////////////////////////////////////////////

    // currently just accepting everything but that will change later
    return nfq_set_verdict(qh, id, NF_ACCEPT, payload_len, packet_data);
  } catch (std::exception &error) {
    std::cerr << "Error processing packet: " << error.what() << "\n";
    return nfq_set_verdict(qh, id, NF_ACCEPT, 0, nullptr);
  }
}