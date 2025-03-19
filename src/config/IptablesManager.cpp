#include "IptablesManager.hpp"
#include "configs.hpp"
#include <iostream>

IptablesManager::IptablesManager() { setupRules(); }

IptablesManager::~IptablesManager() {
  try {
    teardownRules();
  } catch (const std::exception &error) {
    std::cerr << "Error tearing down iptables rules: " << error.what() << "\n";
  }
}

void IptablesManager::setupRules() {
  std::cout << "Setting up iptables rules for " << WG_INTERFACE << ".\n";

  // Forward incoming wireguard traffic to nfqueue
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

  std::cout << "iptables rules set up successfully." << "\n";
}

void IptablesManager::teardownRules() {
  std::cout << "Tearing down iptables rules..." << "\n";

  bool success = true;

  // Remove incoming rule
  try {
    executeCommand("iptables -D FORWARD -i " + WG_INTERFACE +
                   " -j NFQUEUE --queue-num " + std::to_string(QUEUE_NUM));
  } catch (const std::exception &error) {
    std::cerr << "Warning: Failed to remove incoming iptables rules: "
              << error.what() << std::endl;
    success = false;
  }

  // Remove outgoing rule
  try {
    executeCommand("iptables -D FORWARD -o " + WG_INTERFACE +
                   " -j NFQUEUE --queue-num " + std::to_string(QUEUE_NUM));
  } catch (const std::exception &error) {
    std::cerr << "Warning: Failed to remove outgoing iptables rules: "
              << error.what() << std::endl;
    success = false;
  }

  if (success) {
    std::cout << "Successfully removed iptables rules" << std::endl;
  }
}

void IptablesManager::executeCommand(const std::string &command) {
  int result = system(command.c_str());
  if (result != 0) {
    throw std::runtime_error("Command failed: " + command +
                             " (exit code: " + std::to_string(result) + ")");
  }
}