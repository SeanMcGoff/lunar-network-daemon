// Packet.hpp

#pragma once

#include <cstdint> // uint32_t
#include <cstddef> // size_t
#include <chrono>


struct Packet
{
    uint32_t id;   // netfilter queue's packet ID is a 32-bit unsigned integer
    uint8_t *data; // netfilter API works with raw packet data as array of bytes
    size_t length;

    // netfilter mark to classify the packet link type
    uint32_t mark;

    std::chrono::steady_clock::time_point send_time;

    Packet(uint32_t id, uint8_t *data, size_t length, uint32_t mark,
        std::chrono::steady_clock::time_point send_time);

    ~Packet();
};

namespace PacketUtils {

    enum class LinkType {
        EARTH_TO_EARTH,
        EARTH_TO_MOON,
        MOON_TO_EARTH,
        MOON_TO_MOON,
        OTHER
    };
    
    bool isRoverIP(uint32_t ip);
    bool isBaseIP(uint32_t ip);
    LinkType classifyPacket(const Packet& pkt);
    bool extractIPs(const Packet& pkt, uint32_t& src_ip, uint32_t& dst_ip);

} // namespace PacketUtils