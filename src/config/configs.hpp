// src/config/configs.hpp

#pragma once

#include <string>

#include "ConfigManager.hpp"

// For reference
// 0 => base_latency_ms;
// 1 => latency_jitter_ms;
// 2 => latency_jitter_stddev;
// 3 => base_bit_error_rate;
// 4 => bit_error_rate_stddev;
// 5 => base_packet_loss_burst_freq_per_hour;
// 6 => packet_loss_burst_freq_stddev;
// 7 => base_packet_loss_burst_duration_ms;
// 8 => base_packet_loss_burst_duration_stddev;
// 9 => throughput_limit_mbps;

constexpr const Config::LinkProperties DEFAULT_EARTH_TO_EARTH{0, 0, 0, 0, 0,
                                                              0, 0, 0, 0, 0};
constexpr const Config::LinkProperties DEFAULT_EARTH_TO_MOON{
    1280.0, 100.0, 50.0, 1e-5, 5e-6, 1.0, 0.5, 500.0, 100.0, 0.0};
constexpr const Config::LinkProperties DEFAULT_MOON_TO_EARTH{
    1280.0, 100.0, 50.0, 1e-5, 5e-6, 1.0, 0.5, 500.0, 100.0, 7.5};
constexpr const Config::LinkProperties DEFAULT_MOON_TO_MOON{
    30.0, 10.0, 5.0, 2e-6, 1e-6, 0.2, 0.1, 50.0, 10.0, 7.5};

constexpr uint32_t ROVER_IP_MIN =
    (10 << 24 | 237 << 16 | 0 << 8 | 2); // minimum is 10.237.0.2
constexpr uint32_t ROVER_IP_MAX =
    (10 << 24 | 237 << 16 | 0 << 8 | 120); // maximum is 10.237.0.120
constexpr uint32_t BASE_IP_MIN =
    (10 << 24 | 237 << 16 | 0 << 8 | 130); // minimum is 10.237.0.130
constexpr uint32_t BASE_IP_MAX =
    (10 << 24 | 237 << 16 | 0 << 8 | 253); // maximum is 10.237.0.253

// Netfilter verdict constants
constexpr int NF_ACCEPT = 1;
constexpr int NF_DROP = 0;

// Netfilter configurations
constexpr int QUEUE_NUM = 0;
constexpr int SOCKET_BUFFER_SIZE = 1024 * 1024; // 1MB socket buffer
constexpr int MAX_PACKET_SIZE = 65536;          // 64KB max packet size

// Interface name
const std::string WG_INTERFACE = "wg0";
