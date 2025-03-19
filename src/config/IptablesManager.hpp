// src/config/IptablesManager.hpp

#pragma once

#include <string>

class IptablesManager {
public:
  IptablesManager();
  ~IptablesManager();

private:
  void setupRules();
  void teardownRules();
  void executeCommand(const std::string &command);
};