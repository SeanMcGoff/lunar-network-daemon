// src/netfilter/NetfilterQueue.hpp

// ---- NetfilterQueue Usage ---- //

// NetfilterQueue interfaces with Linux's netfilter library
// it captures packets that have been directed to NFQUEUE by iptables rules

// Example:
// NetfilterQueue queue;
// queue.run(); // this blocks until queue.stop() is called from a signal

// in main, queue is a global pointer, instantiate using std::make_unique

// the run() method has an internal loop processing packets as they arrive
// Each packet triggers the packetCallback method, this is where the processing
// pipeline will be called

// if modifying this class:
// - packetCallbackStatic is needed for C++ to C callback conversion
// - smart pointers should handle resource cleanup automatically
// - the socket buffer size is configurable in configs.hpp

#pragma once

#include <atomic>
#include <csignal>
#include <functional>
#include <memory>

#include <libnetfilter_queue/libnetfilter_queue.h>

class NetfilterQueue {
  // NOTE: I'm using the very obtuse netfilter naming conventions for
  // convenience (easier to follow examples), I've explained it all (mostly) at
  // the bottom
public:
  NetfilterQueue();
  void run();
  void stop();
  bool isRunning() const;

private:
  // this is a "static bridge" pattern which is required for interfacing C++
  // logic with C libraries that use callbacks
  // The static callback has the exact signature the C libary expects, it
  // receives the "this" pointer through the data parameter,
  // Acting as a bridge to the actual instance method
  static int packetCallbackStatic(struct nfq_q_handle *qh,
                                  struct nfgenmsg *nfmsg, struct nfq_data *nfa,
                                  void *data);

  // The actual callback method has access to all the object's members and state
  int packetCallback(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
                     struct nfq_data *nfa);

  // file descriptor for netlink socket
  int fd_;

  // thread safe flag for controlling the processing loop
  std::atomic<bool> running_;

  // smart pointers for resource management
  std::unique_ptr<struct nfq_handle, decltype(&nfq_close)> handle_;
  std::unique_ptr<struct nfq_q_handle,
                  std::function<void(struct nfq_q_handle *)>>
      queue_handle_;
};

// netfilter

// I'm using the slightly obtuse netfilter naming conventions to make it easy
// to follow the documentation. I'll try to explain the meaning of all below:

// nfq_handle (nfq_h)
// purpose => main handle to netfilter queue subsystem.
// lifecycle => created with nfq_open() and destroyed with nfq_close().
// usage => required for all netfilter operations.

// nfq_q_handle (nfq_qh or qh)
// purpose => handle to specific netfilter queue
// lifecycle => create w/ nfq_create_queue(), destroy w/ nfq_destroy_queue()
// usage => use it to interact with a specific queue

// nfgenmsg
// purpose => generic netlink message header
// usage => passed to callback functions, contains protocol information
// fields => contains IP family (ipv4, ipv6) and version info
// explanation =>
//    part of linux messaging system, struct contains metadata about
//    the netlink message itself

// nfq_data
// purpose => container for all packet-related data
// usage =>
//    passed to callback funcs, used to extract packet metadata and
//    packet payload
//    int nfq_get_payload(struct nfq_data *nfad, unsigned char **data)

// nfq_fd
// purpose => file descriptor for the netfilter queue connection,
//    it serves like a socket that serves as the comms channel
//    between the userspace app and the kernel
// usage => read from the file descriptor using standard socket operations
//    such as recv(), select(), poll(), or epoll()