// src/config/ConfigManager.cpp

#include "ConfigManager.hpp"
#include "configs.hpp"
#include <fstream>
#include <iostream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

// Anonymous namespace (to avoid cluttering global namespace)
namespace {
namespace nm = nlohmann;

// Helper functions
double getDoubleWithLog(const nm::json &j, const std::string &key,
                        const double defaultValue);
bool loadSectionExists(const nm::json &j, const std::string &section);
void loadLinkProperties(const nm::json &j, Config::LinkProperties &props,
                        const Config::LinkProperties &defaults);
void loadSection(const nm::json &j, const std::string &sectionName,
                 Config::LinkProperties &target,
                 const Config::LinkProperties &defaults);
} // namespace

ConfigManager::ConfigManager(const std::string &config_file)
    : config_file_(config_file) {
  try {
    loadConfig();
  } catch (const std::exception &error) {
    spdlog::warn("No previous configuration available: {}. Using default "
                 "configuration.",
                 error.what());
    loadDefaultConfig();
  }
}

Config ConfigManager::getConfig() {
  // shared lock, multiple threads can read concurrently
  std::shared_lock<std::shared_mutex> lock(config_mutex_);
  return config_;
}

Config::LinkProperties ConfigManager::getEToEConfig() {
  // shared lock, multiple threads can read concurrently
  std::shared_lock<std::shared_mutex> lock(config_mutex_);
  return config_.earth_to_earth;
}

Config::LinkProperties ConfigManager::getEToMConfig() {
  // shared lock, multiple threads can read concurrently
  std::shared_lock<std::shared_mutex> lock(config_mutex_);
  return config_.earth_to_moon;
}

Config::LinkProperties ConfigManager::getMToEConfig() {
  // shared lock, multiple threads can read concurrently
  std::shared_lock<std::shared_mutex> lock(config_mutex_);
  return config_.moon_to_earth;
}

Config::LinkProperties ConfigManager::getMToMConfig() {
  // shared lock, multiple threads can read concurrently
  std::shared_lock<std::shared_mutex> lock(config_mutex_);
  return config_.moon_to_moon;
}

void ConfigManager::reloadConfig() {
  // Use an exclusive lock while updating the configuration.
  std::unique_lock<std::shared_mutex> lock(config_mutex_);
  try {
    loadConfig();
  } catch (const std::exception &error) {
    spdlog::error("Reload failed: {}. Keeping previous configuration.",
                  error.what());
  }
}

void ConfigManager::loadConfig() {
  std::ifstream infile(config_file_);
  if (!infile) {
    spdlog::critical("Error opening config file: {}", config_file_);
    throw std::runtime_error("Error opening config file: " + config_file_);
  }

  try {
    nm::json j;
    infile >> j;

    loadSection(j, "earth_to_earth", config_.earth_to_earth,
                DEFAULT_EARTH_TO_EARTH);
    loadSection(j, "earth_to_moon", config_.earth_to_moon,
                DEFAULT_EARTH_TO_MOON);
    loadSection(j, "moon_to_earth", config_.moon_to_earth,
                DEFAULT_MOON_TO_EARTH);
    loadSection(j, "moon_to_moon", config_.moon_to_moon, DEFAULT_MOON_TO_MOON);
  } catch (const std::exception &error) {
    spdlog::critical("Error parsing config file: {}. Using previous "
                     "configuration if available. Note: this error may occur "
                     "while editing the config file "
                     "manually.",
                     error.what());
    throw;
  }
}

void ConfigManager::loadDefaultConfig() {
  config_.earth_to_earth = DEFAULT_EARTH_TO_EARTH;
  config_.earth_to_moon = DEFAULT_EARTH_TO_MOON;
  config_.moon_to_earth = DEFAULT_MOON_TO_EARTH;
  config_.moon_to_moon = DEFAULT_MOON_TO_MOON;
}

// ---- Helper function implementations ---- //

namespace {

// Helper function: if key is missing, log and return default.
double getDoubleWithLog(const nm::json &j, const std::string &key,
                        const double defaultValue) {
  if (!j.contains(key)) {
    spdlog::warn("Key '{}' not found, using default {}.", key, defaultValue);
    return defaultValue;
  }
  return j.value(key, defaultValue);
}

// Helper function: Load a section if available, else log an error.
bool loadSectionExists(const nm::json &j, const std::string &section) {
  if (!j.contains(section)) {
    spdlog::error("Section '{}' not found in configuration.", section);
    return false;
  }
  return true;
}

// Helper function: Load link properties from a json section using default
// values
void loadLinkProperties(const nm::json &j, Config::LinkProperties &props,
                        const Config::LinkProperties &defaults) {
  props.base_latency_ms =
      getDoubleWithLog(j, "base_latency_ms", defaults.base_latency_ms);
  props.latency_jitter_ms =
      getDoubleWithLog(j, "latency_jitter_ms", defaults.latency_jitter_ms);
  props.latency_jitter_stddev = getDoubleWithLog(
      j, "latency_jitter_stddev", defaults.latency_jitter_stddev);
  props.base_bit_error_rate =
      getDoubleWithLog(j, "base_bit_error_rate", defaults.base_bit_error_rate);
  props.bit_error_rate_stddev = getDoubleWithLog(
      j, "bit_error_rate_stddev", defaults.bit_error_rate_stddev);
  props.base_packet_loss_burst_freq_per_hour =
      getDoubleWithLog(j, "base_packet_loss_burst_freq_per_hour",
                       defaults.base_packet_loss_burst_freq_per_hour);
  props.packet_loss_burst_freq_stddev =
      getDoubleWithLog(j, "packet_loss_burst_freq_stddev",
                       defaults.packet_loss_burst_freq_stddev);
  props.base_packet_loss_burst_duration_ms =
      getDoubleWithLog(j, "base_packet_loss_burst_duration_ms",
                       defaults.base_packet_loss_burst_duration_ms);
  props.base_packet_loss_burst_duration_stddev =
      getDoubleWithLog(j, "base_packet_loss_burst_duration_stddev",
                       defaults.base_packet_loss_burst_duration_stddev);
  props.throughput_limit_mbps = getDoubleWithLog(
      j, "throughput_limit_mbps", defaults.throughput_limit_mbps);
}

// Helper function: Load a configuration section
void loadSection(const nm::json &j, const std::string &sectionName,
                 Config::LinkProperties &target,
                 const Config::LinkProperties &defaults) {
  if (loadSectionExists(j, sectionName)) {
    auto &sec = j[sectionName];
    loadLinkProperties(sec, target, defaults);
  } else {
    spdlog::critical("Section '{}' missing in config file.", sectionName);
    throw std::runtime_error(sectionName + " section missing in config file.");
  }
}
} // namespace
