#include "IptablesManager.hpp"
#include "configs.hpp"
#include <iostream>

IptablesManager::IptablesManager() {
std::cout << "Setting up iptables rules for " << WG_INTERFACE << ".\n";

  // Forward wireguard traffic to nfqueue
  // -A FORWARD: Append a rule to the FORWARD chain (ie. packets being routed
  // through this host) -i wg0: Match packets whose incoming (-i meaning
  // incoming) interface is wg0 -j NFQUEUE: "Jump" to the NFQUEUE target (ie.
  // hand off to NFQUEUE instead of dropping or accepting)
  // --queue-num 0: Put packets into queue number 0.
  executeCommand("iptables -A FORWARD -i " + WG_INTERFACE +
                 " -j NFQUEUE --queue-num " + std::to_string(QUEUE_NUM));

  try {
    // Forward outgoing wireguard traffic to nfqueue
    executeCommand("iptables -A FORWARD -o " + WG_INTERFACE +
                   " -j NFQUEUE --queue-num " + std::to_string(QUEUE_NUM));
  } catch (const std::exception &error) {
    // Clean up if second rule fails
    executeCommand("iptables -D FORWARD -i " + WG_INTERFACE +
                   " -j NFQUEUE --queue-num " + std::to_string(QUEUE_NUM));
    throw;
    }

IptablesManager::~IptablesManager() {
  try {
  std::cout << "Tearing down iptables rules..." << "\n";

  bool success = true;

  // Remove incoming rule
  try {
    executeCommand("iptables -D FORWARD -i " + WG_INTERFACE +
                   " -j NFQUEUE --queue-num " + std::to_string(QUEUE_NUM));
  } catch (const std::exception &error) {
    std::cerr << "Warning: Failed to remove incoming iptables rules: "
              << error.what() << "\n";
    success = false;
  }

  // Remove outgoing rule
  try {
    executeCommand("iptables -D FORWARD -o " + WG_INTERFACE +
                   " -j NFQUEUE --queue-num " + std::to_string(QUEUE_NUM));
  } catch (const std::exception &error) {
    std::cerr << "Warning: Failed to remove outgoing iptables rules: "
              << error.what() << "\n";
    success = false;
  }

  if (success) {
    std::cout << "Successfully removed iptables rules.\n";
  }
  } catch (const std::exception &error) {
    std::cerr << "Error tearing down iptables rules: " << error.what() << "\n";
  }
}


void IptablesManager::executeCommand(const std::string &command) {
  int result = system(command.c_str());
  if (result != 0) {
    throw std::runtime_error("Command failed: " + command +
                             " (exit code: " + std::to_string(result) + ")");
  }
}