// src/config/TcNetemManager.cpp

#include "TcNetemManager.hpp"
#include "configs.hpp"
#include <exception>
#include <iostream>
#include <string>

TcNetemManager::TcNetemManager(const ConfigManager &config_manager) {
  std::cout << "Setting up TC/Netem rules for " << WG_INTERFACE << ".\n";
  try {
    setupTcRules(config_manager);
  } catch (const std::exception &error) {
    std::cerr << "Error setting up TC/Netem rules: " << error.what() << "\n";
    teardownTcRules();
    throw;
  }
}

void TcNetemManager::executeCommand(const std::string &command) {
  std::cout << "Executing: " << command << "\n";
  int result = system(command.c_str());
  if (result != 0) {
    throw std::runtime_error("Command failed: " + command +
                             " (exit code: " + std::to_string(result) + ")");
  }
}

void TcNetemManager::setupTcRules(const ConfigManager &config_manager) {
  // get configurations
  Config config = config_manager.getConfig();

  // make sure netem is running
  executeCommand("modprobe sch_netem");

  // In another life, we would limit the throughput on a rover by rover basis
  // But fuck that
  const std::string default_rate = "1000Mbit";

  // Create the root qdisc for outgoing traffic on wg0
  // Default to class 1 (EARTH_TO_EARTH) for unclassified traffic
  // qdisc is short for queueing discipline
  // classes can be attached to the qdisc
  executeCommand("tc qdisc add dev " + WG_INTERFACE +
                 " root handle 1: htb default " +
                 std::to_string(MARK_EARTH_TO_EARTH));

  // Create classes for each link type
  executeCommand("tc class add dev " + WG_INTERFACE +
                 " parent 1: classid 1:" + std::to_string(MARK_EARTH_TO_EARTH) +
                 " htb rate " + default_rate + " ceil " + default_rate);
  executeCommand("tc class add dev " + WG_INTERFACE +
                 " parent 1: classid 1:" + std::to_string(MARK_EARTH_TO_MOON) +
                 " htb rate " + default_rate + " ceil " + default_rate);
  executeCommand("tc class add dev " + WG_INTERFACE +
                 " parent 1: classid 1:" + std::to_string(MARK_MOON_TO_EARTH) +
                 " htb rate " + default_rate + " ceil " + default_rate);
  executeCommand("tc class add dev " + WG_INTERFACE +
                 " parent 1: classid 1:" + std::to_string(MARK_MOON_TO_MOON) +
                 " htb rate " + default_rate + " ceil " + default_rate);

  // Example command:
  // tc qdisc add dev wg0 parent 1:1 handle 10: netem delay 1300ms 50ms 0%

  // EARTH_TO_EARTH
  executeCommand("tc qdisc add dev " + WG_INTERFACE +
                 " parent 1:" + std::to_string(MARK_EARTH_TO_EARTH) +
                 " handle 10: netem delay " +
                 std::to_string(config.earth_to_earth.base_latency_ms) + "ms " +
                 std::to_string(config.earth_to_earth.latency_jitter_ms) +
                 "ms 0%");

  // EARTH_TO_MOON
  executeCommand("tc qdisc add dev " + WG_INTERFACE +
                 " parent 1:" + std::to_string(MARK_EARTH_TO_MOON) +
                 " handle 20: netem delay " +
                 std::to_string(config.earth_to_moon.base_latency_ms) + "ms " +
                 std::to_string(config.earth_to_moon.latency_jitter_ms) +
                 "ms 0%");

  // MOON_TO_EARTH
  executeCommand("tc qdisc add dev " + WG_INTERFACE +
                 " parent 1:" + std::to_string(MARK_MOON_TO_EARTH) +
                 " handle 30: netem delay " +
                 std::to_string(config.moon_to_earth.base_latency_ms) + "ms " +
                 std::to_string(config.moon_to_earth.latency_jitter_ms) +
                 "ms 0%");

  // MOON_TO_MOON
  executeCommand("tc qdisc add dev " + WG_INTERFACE +
                 " parent 1:" + std::to_string(MARK_MOON_TO_MOON) +
                 " handle 40: netem delay " +
                 std::to_string(config.moon_to_moon.base_latency_ms) + "ms " +
                 std::to_string(config.moon_to_moon.latency_jitter_ms) +
                 "ms 0%");

  // Add filters to match packets based on netfilter marks
  executeCommand("tc filter add dev " + WG_INTERFACE +
                 " parent 1: protocol ip prio 1 handle " +
                 std::to_string(MARK_EARTH_TO_EARTH) +
                 " fw flowid 1:" + std::to_string(MARK_EARTH_TO_EARTH));
  executeCommand("tc filter add dev " + WG_INTERFACE +
                 " parent 1: protocol ip prio 1 handle " +
                 std::to_string(MARK_EARTH_TO_MOON) +
                 " fw flowid 1:" + std::to_string(MARK_EARTH_TO_MOON));
  executeCommand("tc filter add dev " + WG_INTERFACE +
                 " parent 1: protocol ip prio 1 handle " +
                 std::to_string(MARK_MOON_TO_EARTH) +
                 " fw flowid 1:" + std::to_string(MARK_MOON_TO_EARTH));
  executeCommand("tc filter add dev " + WG_INTERFACE +
                 " parent 1: protocol ip prio 1 handle " +
                 std::to_string(MARK_MOON_TO_MOON) +
                 " fw flowid 1:" + std::to_string(MARK_MOON_TO_MOON));
}

void TcNetemManager::teardownTcRules() {
  try {
    executeCommand("tc qdisc del dev " + WG_INTERFACE + " root");
  } catch (const std::exception &error) {
    std::cerr << "Warning: Failed to remove TC rules: " << error.what() << "\n";
  }
}