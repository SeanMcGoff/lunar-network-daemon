// src/netfilter/NetfilterManager.hpp
#pragma once

#include <atomic> // std::atomic<bool>
#include <chrono>
#include <functional> // std::function
#include <string>

#include <libnetfilter_queue/libnetfilter_queue.h>

class NetfilterManager {
public:
  // type definition for packet handling callback.
  // when a NetfilterManager instance is created, the user will provide a
  // callback function that matches this signature
  using PacketHandlerFunc = std::function<void(
      uint32_t id, const uint8_t *data, size_t length, uint32_t mark,
      std::chrono::steady_clock::time_point time_received)>;

  NetfilterManager(const std::string &interface_name, PacketHandlerFunc handler,
                   int queue_num = 0);

  ~NetfilterManager();

private:
  // configuration
  std::string interface_name_;
  PacketHandlerFunc handler_;
  int queue_num_;

  // state
  bool initialised_;
  std::atomic<bool> running_;

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

  struct nfq_handle *nfq_h_;
  struct nfq_q_handle *nfq_qh_;
  int nfq_fd;

  // static callback for netfilter
  static int packetCallback(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
                            struct nfq_data *nfad, void *data);
};