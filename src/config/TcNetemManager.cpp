// src/config/TcNetemManager.cpp

#include "TcNetemManager.hpp"
#include "configs.hpp"
#include <iostream>
#include <string>

void TcNetemManager::executeCommand(const std::string &command) {
  std::cout << "Executing: " << command << "\n;";
  int result = system(command.c_str());
  if (result != 0) {
    throw std::runtime_error("Command failed: " + command +
                             " (exit code: " + std::to_string(result) + ")");
  }
}

void TcNetemManager::setupTcRules(const ConfigManager &config_manager) {
  // get configurations
  Config config = config_manager.getConfig();

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
}