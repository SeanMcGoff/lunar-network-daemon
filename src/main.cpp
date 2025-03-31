// src/main.cpp

#include <csignal>
#include <cstring>
#include <exception>
#include <iostream>

// netfilter includes
#include <asm-generic/socket.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <libnetfilter_queue/linux_nfnetlink_queue.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "ConfigManager.hpp"
#include "IptablesManager.hpp"
#include "NetfilterQueue.hpp"
#include "TcNetemManager.hpp"
#include "configs.hpp"

void signalHandler(int signal);
void setupSignalHandlers();

std::unique_ptr<NetfilterQueue> g_queue;

int main() {
  std::cout << "Starting packet interception on " << WG_INTERFACE << "\n";

  try {
    setupSignalHandlers();

    // create config manager
    ConfigManager config_manager("config/config.json");

    // iptables class ensures teardown on destruction
    IptablesManager iptables;

    // Set up TC/Netem rules, torn down on destruction
    TcNetemManager tc_netem(config_manager);

    g_queue = std::make_unique<NetfilterQueue>(config_manager);

    // blocks until stopped by signal
    g_queue->run();

    std::cout << "Shutting down.\n";

    // Clean up the queue
    g_queue.reset();

  } catch (const std::exception &error) {
    std::cout << "Fatal error: " << error.what() << "\n";
  }

  std::cout << "Shutdown complete.\n";
  return 0;
}

void signalHandler(int signal) {
  if (g_queue) {
    g_queue->stop();
  }
}

void setupSignalHandlers() {
  struct sigaction sa{};
  std::memset(&sa, 0, sizeof(sa));
  sa.sa_handler = signalHandler;

  // Register for common termination signals
  if (sigaction(SIGINT, &sa, nullptr) < 0) { // Ctrl+C
    std::cerr << "Warning: Failed to set SIGINT handler\n";
  }
  if (sigaction(SIGTERM, &sa, nullptr) < 0) { // Termination signal
    std::cerr << "Warning: Failed to set SIGTERM handler\n";
  }
  if (sigaction(SIGHUP, &sa, nullptr) < 0) { // Terminal closed
    std::cerr << "Warning: Failed to set SIGHUP handler\n";
  }
}