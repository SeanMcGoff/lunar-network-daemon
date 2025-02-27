// ConfigManager.cpp

#include "ConfigManager.hpp"
#include <nlohmann/json.hpp>
#include <mutex>
#include <fstream>
#include <iostream>

// Anonymous namespace (to avoid cluttering global namespace)
namespace
{

    namespace nm = nlohmann;

    // Helper function: if key is missing, log and return default.
    double getDoubleWithLog(const nm::json &j, const std::string &key, const double defaultValue)
    {
        if (!j.contains(key))
        {
            std::cerr << "Key '" << key << "' not found, using default " << defaultValue << ".\n";
            return defaultValue;
        }
        return j.value(key, defaultValue);
    }

    // Helper function: Load a section if available, else log an error.
    bool loadSectionExists(const nm::json &j, const std::string &section)
    {
        if (!j.contains(section))
        {
            std::cerr << "Section '" << section << "' not found in configuration.\n";
            return false;
        }
        return true;
    }
}

ConfigManager::ConfigManager(const std::string &config_file) : config_file_(config_file)
{
    try
    {
        loadConfig();
    }
    catch (const std::exception &error)
    {
        std::cerr << "No previous configuration available: " << error.what()
                  << "\nUsing default configuration.\n";
        loadDefaultConfig();
    }
}

Config ConfigManager::getConfig()
{
    // shared lock, multiple threads can read concurrently
    std::shared_lock<std::shared_mutex> lock(config_mutex_);
    return config_;
}

void ConfigManager::reloadConfig()
{
    // Use an exclusive lock while updating the configuration.
    std::unique_lock<std::shared_mutex> lock(config_mutex_);
    try
    {
        loadConfig();
    }
    catch (const std::exception &error)
    {
        std::cerr << "Reload failed: " << error.what()
                  << "\nKeeping previous configuration.\n";
    }
}

void ConfigManager::loadConfig()
{
    std::ifstream infile(config_file_);
    if (!infile)
    {
        std::cerr << "Error opening config file: " << config_file_
                  << "\nUsing previous configuration if available.\n";
        throw std::runtime_error("Error opening config file: " + config_file_);
    }

    try
    {
        nm::json j;
        infile >> j;

        if (loadSectionExists(j, "earth_to_earth"))
        {
            auto &sec = j["earth_to_earth"];
            config_.earth_to_earth.base_latency_ms = getDoubleWithLog(sec, "base_latency_ms", 0);
            config_.earth_to_earth.latency_jitter_ms = getDoubleWithLog(sec, "latency_jitter_ms", 0);
            config_.earth_to_earth.latency_jitter_stddev = getDoubleWithLog(sec, "latency_jitter_stddev", 0);
            config_.earth_to_earth.base_bit_error_rate = getDoubleWithLog(sec, "base_bit_error_rate", 0);
            config_.earth_to_earth.bit_error_rate_stddev = getDoubleWithLog(sec, "bit_error_rate_stddev", 0);
            config_.earth_to_earth.base_packet_loss_burst_freq_per_hour = getDoubleWithLog(sec, "base_packet_loss_burst_freq_per_hour", 0);
            config_.earth_to_earth.packet_loss_burst_freq_stddev = getDoubleWithLog(sec, "packet_loss_burst_freq_stddev", 0);
            config_.earth_to_earth.base_packet_loss_burst_duration_ms = getDoubleWithLog(sec, "base_packet_loss_burst_duration_ms", 0);
            config_.earth_to_earth.base_packet_loss_burst_duration_stddev = getDoubleWithLog(sec, "base_packet_loss_burst_duration_stddev", 0);
            config_.earth_to_earth.throughput_limit_mbps = getDoubleWithLog(sec, "throughput_limit_mbps", 0);
        } else throw std::runtime_error("Earth to earth section missing in config file.");

        // Earth to moon
        if (loadSectionExists(j, "earth_to_moon"))
        {
            auto &sec = j["earth_to_moon"];
            config_.earth_to_moon.base_latency_ms = getDoubleWithLog(sec, "base_latency_ms", 1280.0);
            config_.earth_to_moon.latency_jitter_ms = getDoubleWithLog(sec, "latency_jitter_ms", 100.0);
            config_.earth_to_moon.latency_jitter_stddev = getDoubleWithLog(sec, "latency_jitter_stddev", 50.0);
            config_.earth_to_moon.base_bit_error_rate = getDoubleWithLog(sec, "base_bit_error_rate", 1e-5);
            config_.earth_to_moon.bit_error_rate_stddev = getDoubleWithLog(sec, "bit_error_rate_stddev", 5e-6);
            config_.earth_to_moon.base_packet_loss_burst_freq_per_hour = getDoubleWithLog(sec, "base_packet_loss_burst_freq_per_hour", 1.0);
            config_.earth_to_moon.packet_loss_burst_freq_stddev = getDoubleWithLog(sec, "packet_loss_burst_freq_stddev", 0.5);
            config_.earth_to_moon.base_packet_loss_burst_duration_ms = getDoubleWithLog(sec, "base_packet_loss_burst_duration_ms", 500.0);
            config_.earth_to_moon.base_packet_loss_burst_duration_stddev = getDoubleWithLog(sec, "base_packet_loss_burst_duration_stddev", 100.0);
            config_.earth_to_moon.throughput_limit_mbps = getDoubleWithLog(sec, "throughput_limit_mbps", 0);
        } else throw std::runtime_error("Earth to moon section missing in config file.");

        // Moon to earth
        if (loadSectionExists(j, "moon_to_earth"))
        {
            auto &sec = j["moon_to_earth"];
            config_.moon_to_earth.base_latency_ms = getDoubleWithLog(sec, "base_latency_ms", 1280.0);
            config_.moon_to_earth.latency_jitter_ms = getDoubleWithLog(sec, "latency_jitter_ms", 100.0);
            config_.moon_to_earth.latency_jitter_stddev = getDoubleWithLog(sec, "latency_jitter_stddev", 50.0);
            config_.moon_to_earth.base_bit_error_rate = getDoubleWithLog(sec, "base_bit_error_rate", 1e-5);
            config_.moon_to_earth.bit_error_rate_stddev = getDoubleWithLog(sec, "bit_error_rate_stddev", 5e-6);
            config_.moon_to_earth.base_packet_loss_burst_freq_per_hour = getDoubleWithLog(sec, "base_packet_loss_burst_freq_per_hour", 1.0);
            config_.moon_to_earth.packet_loss_burst_freq_stddev = getDoubleWithLog(sec, "packet_loss_burst_freq_stddev", 0.5);
            config_.moon_to_earth.base_packet_loss_burst_duration_ms = getDoubleWithLog(sec, "base_packet_loss_burst_duration_ms", 500.0);
            config_.moon_to_earth.base_packet_loss_burst_duration_stddev = getDoubleWithLog(sec, "base_packet_loss_burst_duration_stddev", 100.0);
            config_.moon_to_earth.throughput_limit_mbps = getDoubleWithLog(sec, "throughput_limit_mbps", 7.5);
        } else throw std::runtime_error("Moon to earth section missing in config file.");

        // Moon to moon
        if (loadSectionExists(j, "moon_to_moon"))
        {
            auto &sec = j["moon_to_moon"];
            config_.moon_to_moon.base_latency_ms = getDoubleWithLog(sec, "base_latency_ms", 30.0);
            config_.moon_to_moon.latency_jitter_ms = getDoubleWithLog(sec, "latency_jitter_ms", 10.0);
            config_.moon_to_moon.latency_jitter_stddev = getDoubleWithLog(sec, "latency_jitter_stddev", 5.0);
            config_.moon_to_moon.base_bit_error_rate = getDoubleWithLog(sec, "base_bit_error_rate", 2e-6);
            config_.moon_to_moon.bit_error_rate_stddev = getDoubleWithLog(sec, "bit_error_rate_stddev", 1e-6);
            config_.moon_to_moon.base_packet_loss_burst_freq_per_hour = getDoubleWithLog(sec, "base_packet_loss_burst_freq_per_hour", 0.2);
            config_.moon_to_moon.packet_loss_burst_freq_stddev = getDoubleWithLog(sec, "packet_loss_burst_freq_stddev", 0.1);
            config_.moon_to_moon.base_packet_loss_burst_duration_ms = getDoubleWithLog(sec, "base_packet_loss_burst_duration_ms", 50.0);
            config_.moon_to_moon.base_packet_loss_burst_duration_stddev = getDoubleWithLog(sec, "base_packet_loss_burst_duration_stddev", 10.0);
            config_.moon_to_moon.throughput_limit_mbps = getDoubleWithLog(sec, "throughput_limit_mbps", 7.5);
        } else throw std::runtime_error("Moon to moon section missing in config file.");
    }
    catch (const std::exception &error)
    {
        std::cerr << "Error parsing config file: " << error.what()
                  << ".\nUsing previous configuration if available.\n"
                  << "Note: this error may occur while editing the config file manually.\n";
        throw;
    }
}

void ConfigManager::loadDefaultConfig()
{
    // Earth to earth
    config_.earth_to_earth.base_latency_ms = 0;
    config_.earth_to_earth.latency_jitter_ms = 0;
    config_.earth_to_earth.latency_jitter_stddev = 0;
    config_.earth_to_earth.base_bit_error_rate = 0;
    config_.earth_to_earth.bit_error_rate_stddev = 0;
    config_.earth_to_earth.base_packet_loss_burst_freq_per_hour = 0;
    config_.earth_to_earth.packet_loss_burst_freq_stddev = 0;
    config_.earth_to_earth.base_packet_loss_burst_duration_ms = 0;
    config_.earth_to_earth.base_packet_loss_burst_duration_stddev = 0;
    config_.earth_to_earth.throughput_limit_mbps = 0;

    // Earth to moon
    config_.earth_to_moon.base_latency_ms = 1280.0;
    config_.earth_to_moon.latency_jitter_ms = 100.0;
    config_.earth_to_moon.latency_jitter_stddev = 50.0;
    config_.earth_to_moon.base_bit_error_rate = 1e-5;
    config_.earth_to_moon.bit_error_rate_stddev = 5e-6;
    config_.earth_to_moon.base_packet_loss_burst_freq_per_hour = 1.0;
    config_.earth_to_moon.packet_loss_burst_freq_stddev = 0.5;
    config_.earth_to_moon.base_packet_loss_burst_duration_ms = 500.0;
    config_.earth_to_moon.base_packet_loss_burst_duration_stddev = 100.0;
    config_.earth_to_moon.throughput_limit_mbps = 0;

    // Moon to earth
    config_.moon_to_earth.base_latency_ms = 1280.0;
    config_.moon_to_earth.latency_jitter_ms = 100.0;
    config_.moon_to_earth.latency_jitter_stddev = 50.0;
    config_.moon_to_earth.base_bit_error_rate = 1e-5;
    config_.moon_to_earth.bit_error_rate_stddev = 5e-6;
    config_.moon_to_earth.base_packet_loss_burst_freq_per_hour = 1.0;
    config_.moon_to_earth.packet_loss_burst_freq_stddev = 0.5;
    config_.moon_to_earth.base_packet_loss_burst_duration_ms = 500.0;
    config_.moon_to_earth.base_packet_loss_burst_duration_stddev = 100.0;
    config_.moon_to_earth.throughput_limit_mbps = 7.5;

    // Moon to moon
    config_.moon_to_moon.base_latency_ms = 30.0;
    config_.moon_to_moon.latency_jitter_ms = 10.0;
    config_.moon_to_moon.latency_jitter_stddev = 5.0;
    config_.moon_to_moon.base_bit_error_rate = 2e-6;
    config_.moon_to_moon.bit_error_rate_stddev = 1e-6;
    config_.moon_to_moon.base_packet_loss_burst_freq_per_hour = 0.2;
    config_.moon_to_moon.packet_loss_burst_freq_stddev = 0.1;
    config_.moon_to_moon.base_packet_loss_burst_duration_ms = 50.0;
    config_.moon_to_moon.base_packet_loss_burst_duration_stddev = 10.0;
    config_.moon_to_moon.throughput_limit_mbps = 7.5;
}
