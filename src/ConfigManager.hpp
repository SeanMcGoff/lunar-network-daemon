// ConfigManager.hpp

#pragma once

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

        // Throughput limit
        double throughput_limit_mbps;
    };

    LinkProperties earth_to_earth;
    LinkProperties earth_to_moon;
    LinkProperties moon_to_earth;
    LinkProperties moon_to_moon;

};
