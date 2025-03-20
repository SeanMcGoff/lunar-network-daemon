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

// Logging
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

void signalHandler(int signal);
void setupSignalHandlers();
void initializeLogger();

std::unique_ptr<NetfilterQueue> g_queue;

int main() {
  initializeLogger();

  std::cout << "Starting packet interception on " << WG_INTERFACE << "\n";
  spdlog::info("Starting packet interception on {}", WG_INTERFACE);

  try {
    setupSignalHandlers();

    // iptables class ensures teardown on destruction
    IptablesManager iptables;

    g_queue = std::make_unique<NetfilterQueue>();

    // blocks until stopped by signal
    g_queue->run();

    // Clean up the queue
    g_queue.reset();

  } catch (const std::exception &error) {
    spdlog::critical("Fatal error: {}", error.what());
  }

  spdlog::info("Shutdown complete.");
  return 0;
}

void initializeLogger() {
  if (!spdlog::get("file_logger")) {
    auto file_logger = spdlog::basic_logger_mt("file_logger", "run.log", true);
    spdlog::set_default_logger(file_logger);
  }

  spdlog::info("Logger initialized successfully.");
}

void signalHandler(int signal) {
  std::cout << "Received signal " << signal << ", shutting down.\n";
  spdlog::info("Received signal {}, shutting down.", signal);
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
    spdlog::warn("Failed to set SIGINT handler");
  }
  if (sigaction(SIGTERM, &sa, nullptr) < 0) { // Termination signal
    spdlog::warn("Failed to set SIGTERM handler");
  }
  if (sigaction(SIGHUP, &sa, nullptr) < 0) { // Terminal closed
    spdlog::warn("Failed to set SIGHUP handler");
  }
}