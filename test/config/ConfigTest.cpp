#include "ConfigManager.hpp"
#include "configs.hpp"

#include <cstring>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

// NOTE: If at any point we need to store pointers in LinkProperties,
// this will most certainly break
bool LinkPropertiesEqual(const Config::LinkProperties& lp1, const Config::LinkProperties& lp2)
{
    return std::memcmp(&lp1, &lp2, sizeof(Config::LinkProperties)) == 0;
}

TEST(LoadDefaultConfig, ConfigTests) {
  // Don't supply config file
  ConfigManager test_config_manager("");
  // Check if config is the same as default config
  auto config = test_config_manager.getConfig();
  EXPECT_TRUE(LinkPropertiesEqual(config.earth_to_earth, DEFAULT_EARTH_TO_EARTH));
  EXPECT_TRUE(LinkPropertiesEqual(config.earth_to_moon, DEFAULT_EARTH_TO_MOON));
  EXPECT_TRUE(LinkPropertiesEqual(config.moon_to_earth, DEFAULT_MOON_TO_EARTH));
  EXPECT_TRUE(LinkPropertiesEqual(config.moon_to_moon, DEFAULT_MOON_TO_MOON));
}
