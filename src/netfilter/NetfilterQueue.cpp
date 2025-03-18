#include "NetfilterQueue.hpp"
#include "configs.hpp"
#include <cerrno>
#include <csignal>
#include <iostream>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <stdexcept>

volatile sig_atomic_t running = true;

NetfilterQueue::NetfilterQueue()
    : handle_(nullptr, nfq_close),
      queue_handle_(nullptr, [](struct nfq_q_handle *qh) {
        if (qh) {
          std::cout << "Destroying queue..." << std::endl;
          nfq_destroy_queue(qh);
        }
      }) {

  std::cout << "Opening Netfilter queue..." << std::endl;

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

  std::cout << "Creating queue and setting callback..." << std::endl;

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
    std::cerr << "Warning: Could not increase socket buffer size" << std::endl;
  }
}

void NetfilterQueue::run() {
  std::cout << "Starting main packet processing loop.\n";

  char buffer[MAX_PACKET_SIZE] __attribute__((aligned));

  while (running) {
    ssize_t received = recv(fd_, buffer, sizeof(buffer), 0);

    if (received >= 0) {
      nfq_handle_packet(handle_.get(), buffer, static_cast<int>(received));
      continue;
    }

    if (errno == ENOBUFS) {
      std::cerr << "Warning: Buffer overflows, packets are being dropped!\n";
      continue;
    }

    if (errno == EINTR) {
      // interrupted by signal, check if we should continue
      if (!running) {
        break;
      }
      continue;
    }

    // any other error
    throw std::runtime_error("recv() failed");
  }

  std::cout << "Exiting main packet processing loop.\n";
}

int NetfilterQueue::packetCallbackStatic(struct nfq_q_handle *qh,
                                         struct nfgenmsg *nfmsg,
                                         struct nfq_data *nfa, void *data) {
  return static_cast<NetfilterQueue *>(data)->packetCallback(qh, nfmsg, nfa);
}