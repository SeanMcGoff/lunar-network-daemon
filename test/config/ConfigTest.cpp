#include "ConfigManager.hpp"
#include "configs.hpp"

#include <gtest/gtest.h>

TEST(ConfigTests, LoadDefaultConfig) {
  // Don't supply config file
  ConfigManager test_config_manager("");
  // Check if config is the same as default config
  auto config = test_config_manager.getConfig();
  EXPECT_EQ(config.earth_to_earth, DEFAULT_EARTH_TO_EARTH);
  EXPECT_EQ(config.earth_to_moon, DEFAULT_EARTH_TO_MOON);
  EXPECT_EQ(config.moon_to_earth, DEFAULT_MOON_TO_EARTH);
  EXPECT_EQ(config.moon_to_moon, DEFAULT_MOON_TO_MOON);
}
