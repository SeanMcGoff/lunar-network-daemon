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

#include "IptablesManager.hpp"
#include "NetfilterQueue.hpp"
#include "configs.hpp"

void setupIPTables();
void teardownIPTables();
void signalHandler(int signal);
void setupSignalHandlers();

int main() {
  std::cout << "Starting packet interception on " << WG_INTERFACE << "\n";

  try {
    setupSignalHandlers();

    // iptables class ensures teardown on destruction
    IptablesManager iptables;

    NetfilterQueue queue;
    queue.run();

  } catch (const std::exception &error) {
    std::cout << "Fatal error: " << error.what() << "\n";
  }

  std::cout << "Shutdown complete.\n";
  return 0;
}

void signalHandler(int signal) {
  std::cout << "Received signal " << signal << ", shutting down.\n";
  running = false;
}

void setupSignalHandlers() {
  struct sigaction sa{};
  std::memset(&sa, 0, sizeof(sa));
  sa.sa_handler = signalHandler;

  sigaction(SIGINT, &sa, nullptr);  // Ctrl+C
  sigaction(SIGTERM, &sa, nullptr); // Termination signal
  sigaction(SIGHUP, &sa, nullptr);  // Terminal closed
}