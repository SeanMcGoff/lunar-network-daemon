// src/config/TcNetemManager.cpp

#include "TcNetemManager.hpp"
#include <iostream>

void TcNetemManager::executeCommand(const std::string &command) {
  std::cout << "Executing: " << command << "\n;";
  int result = system(command.c_str());
  if (result != 0) {
    throw std::runtime_error("Command failed: " + command +
                             " (exit code: " + std::to_string(result) + ")");
  }
}