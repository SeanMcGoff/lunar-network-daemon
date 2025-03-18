#include <iostream>
#include <stdexcept>

#include "ConfigManager.hpp"
#include "Packet.hpp"

int main() {
  // Just testing everything links properly
  ConfigManager manager("config/config.json");
  std::cout << manager.getConfig().earth_to_moon.base_bit_error_rate
            << std::endl;
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
    system("iptables -D FORWARD -i wg0 -j NFQUEUE --queue-num 0");
    throw std::runtime_error(
        "Failed to set up forward iptables rule for outgoing traffic.");
  }

  std::cout << "iptables rules set up successfully\n";
}
