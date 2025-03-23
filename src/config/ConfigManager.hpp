// src/config/ConfigManager.hpp

// ---- ConfigManager Usage ---- //

// The constructor will attempt to load values from a JSON file.
// If no config file is found, it will use the default values.
// Example:
// ConfigManager mgr = ConfigManager("config/config.json");

// getConfig() is a thread-safe getter that returns a copy of config_
// which can then be used or cached.
// Example:
// Config config_copy = mgr.getConfig();

// Other thread-safe getters can also be used for exact link properties.
// Example:
// Config::LinkProperties m_to_e = mgr.getMToEConfig();

// reloadConfig() can be called when the user wants to update
// the config values from the JSON file during runtime.
// Example:
// mgr.reloadConfig();

#pragma once

#include <shared_mutex>
#include <string>

struct Config {
  struct LinkProperties {
    // Latency params (ms)
    double base_latency_ms;
    double latency_jitter_ms;
    double latency_jitter_stddev;

    // Bit error rate params
    double base_bit_error_rate;
    double bit_error_rate_stddev;

    // Packet loss burst params
    double base_packet_loss_burst_freq_per_hour;
    double packet_loss_burst_freq_stddev;
    double base_packet_loss_burst_duration_ms;
    double base_packet_loss_burst_duration_stddev;

    // Throughput limit (note: 0 = no limit)
    double throughput_limit_mbps;

    auto operator<=>(const LinkProperties &) const = default;
  };

  LinkProperties earth_to_earth;
  LinkProperties earth_to_moon;
  LinkProperties moon_to_earth;
  LinkProperties moon_to_moon;
};

class ConfigManager {
public:
  ConfigManager(const std::string &config_file);

  Config getConfig() const;
  Config::LinkProperties getEToEConfig();
  Config::LinkProperties getEToMConfig();
  Config::LinkProperties getMToEConfig();
  Config::LinkProperties getMToMConfig();

  // updates config_ with values from config file
  void reloadConfig();

private:
  std::string config_file_;
  Config config_;

  // Shared mutex allows multiple readers but exclusive write access
  mutable std::shared_mutex config_mutex_;

  void loadConfig();
  void loadDefaultConfig();
};
