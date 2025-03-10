// src/netfilter/NetfilterManager.cpp
#include "NetfilterManager.hpp"
#include <asm-generic/errno-base.h> // errno
#include <asm-generic/errno.h>      // errno
#include <cstring>                  // strerror
#include <iostream>

#include <fcntl.h> // F_GETFL, F_SETFL, O_NONBLOCK
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <sys/socket.h> // recv

NetfilterManager::NetfilterManager(const std::string &interface_name,
                                   PacketHandlerFunc handler, int queue_num)
    : interface_name_(interface_name), handler_(handler), queue_num_(queue_num),
      initialised_(false), running_(false), nfq_h_(nullptr), nfq_qh_(nullptr),
      nfq_fd_(-1) {}

NetfilterManager::~NetfilterManager() {}

bool NetfilterManager::initialise() {
  if (initialised_)
    return true;

  std::cout << "Initialising NetfilterManager for interface " << interface_name_
            << " with queue number " << queue_num_ << "\n";

  try {
    // Open netfilter queue connection
    nfq_h_ = nfq_open();
    if (!nfq_h_) {
      throw std::runtime_error("Error opening netfilter queue handle");
    }

    // Unbind any existing queue from AF_INET (IPv4)
    // makes sure we have a clean slate between builds
    if (nfq_unbind_pf(nfq_h_, AF_INET) < 0) {
      throw std::runtime_error("Error unbinding from IPv4");
    }

    // Bind to AF_INET
    if (nfq_bind_pf(nfq_h_, AF_INET) < 0) {
      throw std::runtime_error("Error binding to IPv4");
    }

    // Create queue instance with out callback function
    // when packets arrive, packetCallback will be called with "this" as context
    nfq_qh_ = nfq_create_queue(nfq_h_, queue_num_,
                               &NetfilterManager::packetCallback, this);
    if (!nfq_qh_) {
      throw std::runtime_error("Error creating netfilter queue " +
                               std::to_string(queue_num_));
    }

    // Set copy_packet mode to get the full packet
    // we need copy mode or the metadata/data wouldn't be available to us
    // 0xFFFF specifies the max size to copy (65535 bytes, the max for IP)
    if (nfq_set_mode(nfq_qh_, NFQNL_COPY_PACKET, 0xFFFF) < 0) {
      throw std::runtime_error("Error setting queue mode");
    }

    // Get the queue file descriptor for polling
    // This fd will be used to read packets from the queue
    nfq_fd_ = nfq_fd(nfq_h_);
    if (nfq_fd_ < 0) {
      throw std::runtime_error("Error getting netfilter queue file descriptor");
    }

    // Set non-blocking mode for the file descriptor
    // First get the current flags
    int flags = fcntl(nfq_fd_, F_GETFL, 0);
    if (flags == -1) {
      throw std::runtime_error(
          std::string("Error getting file descriptor flags: ") +
          strerror(errno));
    }

    // Add non-blocking flag (O_NONBLOCK) to existing flags
    // This allows us to poll the fd without blocking indefinitely
    if (fcntl(nfq_fd_, F_SETFL, flags | O_NONBLOCK) == -1) {
      throw std::runtime_error(
          std::string("Error setting non-blocking mode: ") + strerror(errno));
    }

    initialised_ = true;
    std::cout << "NetfilterManager initialization complete" << std::endl;
    return true;

  } catch (const std::exception &error) {
    std::cerr << "Initialization failed: " << error.what() << std::endl;

    // Clean up any resources that were allocated
    if (nfq_qh_) {
      nfq_destroy_queue(nfq_qh_);
      nfq_qh_ = nullptr;
    }

    if (nfq_h_) {
      nfq_close(nfq_h_);
      nfq_h_ = nullptr;
    }

    nfq_fd_ = -1;
    initialised_ = false;
    return false;
  }
}

bool NetfilterManager::processPackets(bool blocking) {
  if (!initialised_) {
    std::cerr << "Error in processPacket: NetfilterManager not initialised.\n";
    return false;
  }

  running_ = true;

  // 64KB big enough for multiple packets in a single recv() call
  // balance between memory usage and performance
  char buffer[65536];

  std::cout << "Starting to process packets from netfilter queue " << queue_num_
            << ".\n";

  while (running_) {
    // recive data from netfilter queue
    int received =
        recv(nfq_fd_, buffer, sizeof(buffer), blocking ? 0 : MSG_DONTWAIT);

    if (received < 0) {
      // Handle various error conitions
      if (errno == EWOULDBLOCK || errno == EAGAIN) {
        // No data available in non-blocking mode, it just means no packets were
        // ready
        if (blocking)
          return true;

        // in blocking mode this shouldn't happen (unless fd was set as
        // non-blocking somewhere else), but if it does, just try again.
        continue;
      } else if (errno == ENOBUFS) {
        // Queue is full, packets are being dropped by the kernel
        // can happen during traffic spikes
        std::cerr << "ENOBUFS from netfilter queue, packets are being dropped "
                     "by the kernel\n";
        continue;
      } else if (errno == EINTR) {
        // interrupted by signal (could be shutdown request)
        if (!running_)
          break;

        // if not interrupted by a signal to stop processing, just continue
        continue;
      } else {
        // unexpected error condition, log and exit the loop
        std::cerr << "Error receiving from netfilter queue. " << queue_num_
                  << ": " << strerror(errno) << "\n";
        running_ = false;
        return false;
      }
    }

    if (received >= 0) {
      // a single recv() call can return multiple packets
      // nfq_handle_packet will parse the buffer and call our callback for each
      // packet found in the buffer
      nfq_handle_packet(nfq_h_, buffer, received);

      // if in non-blocking mode, return after processing one iteration
      if (!blocking)
        break;
    }

    std::cout << "Processed packets from netfilter queue.\n";
    return true;
  }
}