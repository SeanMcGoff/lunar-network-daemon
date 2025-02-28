#pragma once

#include <cstdint> // uint32_t
#include <cstddef> // size_t
#include <chrono>

namespace
{
    using clk = std::chrono::steady_clock;
}

struct QueuedPacket
{
    uint32_t id;   // netfilter queue's packet ID is a 32-bit unsigned integer
    uint8_t *data; // netfilter API works with raw packet data as array of bytes
    size_t length;

    // netfilter mark to classify the packet link type
    uint32_t mark;

    clk::time_point send_time;

    QueuedPacket(uint32_t id, uint8_t *data, size_t length, uint32_t mark,
                 clk::time_point send_time);

    ~QueuedPacket();
};
