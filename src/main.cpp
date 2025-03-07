#include <iostream>

#include "ConfigManager.hpp"
#include "Packet.hpp"

int main()
{
    // Just testing everything links properly
    ConfigManager manager("config/config.json");
    std::cout << manager.getConfig().earth_to_moon.base_bit_error_rate << std::endl;
}