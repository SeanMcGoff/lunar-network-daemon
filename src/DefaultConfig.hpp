#pragma once

#include "ConfigManager.hpp"

constexpr const Config::LinkProperties DEFAULT_EARTH_TO_EARTH{0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
constexpr const Config::LinkProperties DEFAULT_EARTH_TO_MOON{1280.0, 100.0, 50.0, 1e-5, 5e-6, 1.0, 0.5, 500.0, 100.0, 0.0};
constexpr const Config::LinkProperties DEFAULT_MOON_TO_EARTH{1280.0, 100.0, 50.0, 1e-5, 5e-6, 1.0, 0.5, 500.0, 100.0, 7.5};
constexpr const Config::LinkProperties DEFAULT_MOON_TO_MOON{30.0, 10.0, 5.0, 2e-6, 1e-6, 0.2, 0.1, 50.0, 10.0, 7.5};

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
