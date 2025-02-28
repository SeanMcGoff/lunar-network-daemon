// Packet.cpp

#include "Packet.hpp"
#include "configs.hpp"

#include <cstring> // memcpy
#include <netinet/in.h>

Packet::Packet(uint32_t id, uint8_t *data, size_t length, uint32_t mark,
               std::chrono::steady_clock::time_point send_time)
    : id(id), length(length), mark(mark), send_time(send_time)
{
    // copy data to take ownership
    this->data = new uint8_t[length];
    std::memcpy(this->data, data, length);
}

Packet::~Packet()
{
    if (data)
    {
        delete[] data;
        data = nullptr;
    }
}

namespace PacketUtils
{
    bool isRoverIP(uint32_t ip)
    {
        return (ip >= ROVER_IP_MIN && ip <= ROVER_IP_MAX);
    }

    bool isBaseIP(uint32_t ip)
    {
        return (ip >= BASE_IP_MIN && ip <= BASE_IP_MAX);
    }

    LinkType classifyPacket(const Packet& pkt)
    {
        uint32_t src_ip, dst_ip;

        if (!extractIPs(pkt, src_ip, dst_ip))
        {
            return LinkType::OTHER;
        }

        bool is_src_rover = isRoverIP(src_ip);
        bool is_src_base = isBaseIP(src_ip);
        bool is_dst_rover = isRoverIP(dst_ip);
        bool is_dst_base = isBaseIP(dst_ip);
        if (is_src_rover && is_dst_rover)
            return LinkType::MOON_TO_MOON;
        else if (is_src_rover && is_dst_base)
            return LinkType::MOON_TO_EARTH;
        else if (is_src_base && is_dst_rover)
            return LinkType::EARTH_TO_MOON;
        else if (is_src_base && is_dst_base)
            return LinkType::EARTH_TO_EARTH;
        else
            return LinkType::OTHER;
    }

    bool extractIPs(const Packet& pkt, uint32_t& src_ip, uint32_t& dst_ip)
    {
        // check if long enough to contain IP header
        // should always be longer than 20 but just in case
        if (pkt.length < 20 || !pkt.data)
            return false;

        uint8_t ip_version = (pkt.data[0] >> 4) & 0xF; // first half byte
        if (ip_version != 4)
            return false;

        // ntohl == network to host (big endian to host endianness)
        src_ip = ntohl(*reinterpret_cast<const uint32_t *>(pkt.data + 12));
        dst_ip = ntohl(*reinterpret_cast<const uint32_t *>(pkt.data + 16));

        return true;
    }
} // namespace PacketUtils