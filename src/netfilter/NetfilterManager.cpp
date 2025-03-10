// src/netfilter/NetfilterManager.cpp
#include "NetfilterManager.hpp"
#include <chrono>
#include <functional> // std::function

class NetfilterManager {
public:
  // type definition for packet handling callback.
  // when a NetfilterManager instance is created, the user will provide a
  // callback function that matches this signature
  using PacketHandlerFunc = std::function<void(
      uint32_t id, const uint8_t *data, size_t length, uint32_t mark,
      std::chrono::steady_clock::time_point time_received)>;

  NetfilterManager(const std::string &interface_name, PacketHandlerFunc handler,
                   int queue_num = 0);

  ~NetfilterManager();
};