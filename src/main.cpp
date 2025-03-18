#include <asm-generic/socket.h>
#include <chrono>
#include <iostream>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <libnetfilter_queue/linux_nfnetlink_queue.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>

#include "ConfigManager.hpp"
#include "Packet.hpp"

#define NF_ACCEPT 1
#define NF_DROP 0

void setupIPTables();
void teardownIPTables();

static int packetCallback(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
                          struct nfq_data *nfa, void *data);

int main() {
  std::cout << "Starting packet interception on wg0.\n";

  struct nfq_handle *h = nullptr;
  struct nfq_q_handle *qh = nullptr;
  bool iptables_set = false;

  try {
    setupIPTables();
    iptables_set = true;
  } catch (const std::exception &error) {
    std::cerr << "Failed to set up iptables rules: " << error.what() << "\n";
    throw;
  }

  try {
    int fd, rv;

    // 64KB
    uint8_t buf[65536] __attribute__((aligned));

    std::cout << "Openign Netfilter queue.\n";

    h = nfq_open();
    if (!h) {
      throw std::runtime_error("Failed to open nfqueue");
    }

    if (nfq_unbind_pf(h, AF_INET) < 0) {
      throw std::runtime_error("Failed to unbind ipv4");
    }

    if (nfq_bind_pf(h, AF_INET) < 0) {
      throw std::runtime_error("Failed to bind ipb4");
    }

    std::cout << "Creating queue and setting callback.\n";

    qh = nfq_create_queue(h, 0, &packetCallback, nullptr);
    if (!qh) {
      throw std::runtime_error("Failed to create netfilter queue.");
    }

    if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xFFFF) < 0) {
      throw std::runtime_error("Failed to set NFQ copy-packet mode.");
    }

    // file descriptor for netlink socket
    fd = nfq_fd(h);

    // increase socket buffer size to avoid packet loss
    int opt = 1024 * 1024; // 1MB buffer
    if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &opt, sizeof(opt)) < 0) {
      std::cerr << "Warning: could not increase socket buffer size.\n";
    }

    std::cout << "Starting main loop.\n";
  } catch (...) {
  }
}

// Function to set up iptables rules
void setupIPTables() {
  int return_val = 0;

  std::cout << "Setting up iptable rules\n";

  // Forward wireguard traffic to nfqueue
  // -A FORWARD: Append a rule to the FORWARD chain (ie. packets being routed
  // through this host) -i wg0: Match packets whose incoming (-i meaning
  // incoming) interface is wg0 -j NFQUEUE: "Jump" to the NFQUEUE target (ie.
  // hand off to NFQUEUE instead of dropping or accepting)
  // --queue-num 0: Put packets into queue number 0.
  return_val = system("iptables -A FORWARD -i wg0 -j NFQUEUE --queue-num 0");
  if (return_val != 0) {
    throw std::runtime_error(
        "Failed to set up forward iptables rule for incoming traffic.");
  }

  // Catch outgoing traffic from wg0
  return_val = system("iptables -A FORWARD -o wg0 -j NFQUEUE --queue-num 0");
  if (return_val != 0) {
    // Clean up the previous rule if this one fails
    return_val = system("iptables -D FORWARD -i wg0 -j NFQUEUE --queue-num 0");
    if (return_val != 0) {
      std::cerr
          << "Failed to clean up iptables rules during exception condition.\n";
    }
    throw std::runtime_error(
        "Failed to set up forward iptables rule for outgoing traffic.");
  }

  std::cout << "iptables rules set up successfully\n";
}

void teardownIPTables() {
  int return_val = 0;
  bool successful = true;

  std::cout << "Tearing down iptables rules.";

  // Remove incoming rule
  return_val = system("iptables -D FORWARD -i wg0 -j NFQUEUE --queue-num 0");
  if (return_val != 0) {
    std::cerr << "Warning: Failed to remove incoming iptables rules.\n";
    successful = false;
  }

  return_val = system("iptables -D FORWARD -o wg0 -j NFQUEUE --queue-num 0");
  if (return_val != 0) {
    std::cerr << "Warning: Failed to remove outgoing iptables rules.\n";
    successful = false;
  }

  if (successful)
    std::cout << "Successfully removed iptables rules.\n";
}

static int packetCallback(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
                          struct nfq_data *nfa, void *data) {
  struct nfqnl_msg_packet_hdr *ph;
  uint32_t id = 0;
  uint32_t mark = 0;
  int ret = 0;
  uint8_t *packet_data = nullptr;

  ph = nfq_get_msg_packet_hdr(nfa);
  if (ph) {
    id = ntohl(ph->packet_id);
  } else {
    return nfq_set_verdict(qh, id, NF_ACCEPT, 0, nullptr);
  }

  mark = nfq_get_nfmark(nfa);

  ret = nfq_get_payload(nfa, &packet_data);
  if (ret < 0) {
    std::cerr << "Error: Could not get packet payload.\n";
    return nfq_set_verdict(qh, id, NF_ACCEPT, 0, nullptr);
  }

  try {
    auto now = std::chrono::steady_clock::now();

    Packet packet(id, packet_data, ret, mark, now, false);

    // Print data classfication
    std::cout << "Packet classification: ";
    switch (packet.getLinkType()) {
    case Packet::LinkType::EARTH_TO_EARTH:
      std::cout << "EARTH_TO_EARTH\n";
      break;
    case Packet::LinkType::EARTH_TO_MOON:
      std::cout << "EARTH_TO_MOON\n";
      break;
    case Packet::LinkType::MOON_TO_EARTH:
      std::cout << "MOON_TO_EARTH\n";
      break;
    case Packet::LinkType::MOON_TO_MOON:
      std::cout << "MOON_TO_MOON";
      break;
    default:
      std::cout << "OTHER\n";
    }
  } catch (const std::exception &error) {
    std::cerr << "Error processing packet: " << error.what() << "\n";
  }

  return nfq_set_verdict(qh, id, NF_ACCEPT, ret, nullptr);
}