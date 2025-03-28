// src/config/TcNetemManager.hpp

#pragma once

#include "ConfigManager.hpp"

class TcNetemManager {
public:
  TcNetemManager(const ConfigManager &config_manager);
  ~TcNetemManager();

private:
  void executeCommand(const std::string &command);
  void setupTcRules(const ConfigManager &config_manager);
  void teardownTcRules();
};