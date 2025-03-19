// src/config/IptablesManager.hpp

// ---- IptablesManager Usage ---- //

// This class manages the iptables rules using RAII
// it sets up rules when constructed and tears them down when destroyed

// Example:
// {
//    IptablesManager iptables;
//    // rules are now active
// } // rules are automatically removed when iptables goes out of scope

// the class creates two forwarding rules
// one for incoming and one for outgoing traffic on wg0

// both rules redirect matching packets to the NFQUEUE

// if any rule setup fails, it will clean up the partial config
// and throw an exception

// create the IptablesManager instance before creating NetfilterQueue

#pragma once

#include <string>

class IptablesManager {
public:
  IptablesManager();
  ~IptablesManager();

private:
  void executeCommand(const std::string &command);
};