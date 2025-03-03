// Packet.cpp

#include "Packet.hpp"
#include "configs.hpp"

#include <cstring>      // memcpy
#include <netinet/in.h> // ntohl

// constructor that takes ownership by copying
Packet::Packet(uint32_t id, uint8_t *data, size_t length, uint32_t mark,
               std::chrono::steady_clock::time_point time_received)
    : id(id), length(length), mark(mark), time_received(time_received), data(nullptr)
{
    // copy data to take ownership
    this->data = new uint8_t[length];
    std::memcpy(this->data, data, length);

    this->link_type = PacketClassifier::classifyPacket(this->data, this->length);
}

// constructor that optionally references external data
Packet::Packet(uint32_t id, const uint8_t *data, size_t length, uint32_t mark,
               std::chrono::steady_clock::time_point time_received, bool copy_data)
    : id(id), length(length), mark(mark), time_received(time_received), data(nullptr), owns_data(copy_data)
{
    if (copy_data)
    {
        // Copy data to take ownership
        this->data = new uint8_t[length];
        std::memcpy(this->data, data, length);
    }
    else
    {
        // Just reference the external data
        this->data = const_cast<uint8_t *>(data);
    }

    this->link_type = PacketClassifier::classifyPacket(this->data, this->length);
}

Packet::Packet(const Packet &other)
    : id(other.id), length(other.length), mark(other.mark), time_received(other.time_received),
      data(nullptr), owns_data(other.owns_data), link_type(other.link_type)
{
    if (other.data && other.length > 0)
    {
        if (owns_data)
        {
            this->data = new uint8_t[other.length];
            std::memcpy(this->data, other.data, other.length);
        }
        else
        {
            this->data = other.data; // Just reference the same data
        }
    }
}

Packet &Packet::operator=(const Packet &other)
{
    if (this != &other)
    {
        if (owns_data && data)
        {
            delete[] data;
            data = nullptr;
        }

        id = other.id;
        length = other.length;
        mark = other.mark;
        time_received = other.time_received;
        link_type = other.link_type;
        owns_data = other.owns_data;

        if (other.data && other.length > 0)
        {
            if (owns_data)
            {
                data = new uint8_t[other.length];
                std::memcpy(data, other.data, other.length);
            }
            else
            {
                data = other.data; // Just reference the same data
            }
        }
    }

    return *this;
}

Packet::Packet(Packet &&other) noexcept
    : id(other.id), length(other.length), mark(other.mark), time_received(other.time_received),
      data(other.data), owns_data(other.owns_data), link_type(other.link_type)
{
    other.data = nullptr;
    other.length = 0;
}

Packet &Packet::operator=(Packet &&other) noexcept
{
    if (this != &other)
    {
        if (owns_data && data)
        {
            delete[] data;
        }

        id = other.id;
        length = other.length;
        mark = other.mark;
        time_received = other.time_received;
        link_type = other.link_type;
        data = other.data;
        owns_data = other.owns_data;

        other.data = nullptr;
        other.length = 0;
    }

    return *this;
}

Packet::~Packet()
{
    if (owns_data && data)
    {
        delete[] data;
        data = nullptr;
    }
}


bool Packet::prepareForModification()
{
    if (!owns_data && data) {
        // Make a copy if we don't own the data
        uint8_t* new_data = new uint8_t[length];
        std::memcpy(new_data, data, length);
        data = new_data;
        owns_data = true;
        return true;
    }
    return owns_data;
}


// Getter method implementations
uint32_t Packet::getId() const
{
    return id;
}

const uint8_t *Packet::getData() const
{
    return data;
}

uint8_t *Packet::getMutableData()
{
    // Make sure we own the data before returning a non-const pointer
    if (prepareForModification()) {
        return data;
    }
    return nullptr;
}

size_t Packet::getLength() const
{
    return length;
}

uint32_t Packet::getMark() const
{
    return mark;
}

void Packet::setMark(uint32_t new_mark)
{
    mark = new_mark;
}

std::chrono::steady_clock::time_point Packet::getSendTime() const
{
    return time_received;
}

Packet::LinkType Packet::getLinkType() const
{
    return link_type;
}

// PacketClassifier implementation
Packet::LinkType PacketClassifier::classifyPacket(const uint8_t *data, size_t length)
{
    uint32_t src_ip, dst_ip;

    if (!extractIPs(data, length, src_ip, dst_ip))
    {
        return Packet::LinkType::OTHER;
    }

    bool is_src_rover = isRoverIP(src_ip);
    bool is_src_base = isBaseIP(src_ip);
    bool is_dst_rover = isRoverIP(dst_ip);
    bool is_dst_base = isBaseIP(dst_ip);

    if (is_src_rover && is_dst_rover)
        return Packet::LinkType::MOON_TO_MOON;
    else if (is_src_rover && is_dst_base)
        return Packet::LinkType::MOON_TO_EARTH;
    else if (is_src_base && is_dst_rover)
        return Packet::LinkType::EARTH_TO_MOON;
    else if (is_src_base && is_dst_base)
        return Packet::LinkType::EARTH_TO_EARTH;
    else
        return Packet::LinkType::OTHER;
}

bool PacketClassifier::isRoverIP(uint32_t ip)
{
    return (ip >= ROVER_IP_MIN && ip <= ROVER_IP_MAX);
}

bool PacketClassifier::isBaseIP(uint32_t ip)
{
    return (ip >= BASE_IP_MIN && ip <= BASE_IP_MAX);
}

bool PacketClassifier::extractIPs(const uint8_t *data, size_t length, uint32_t &src_ip, uint32_t &dst_ip)
{
    // check if long enough to contain IP header
    // should always be longer than 20 but just in case
    if (length < 20 || !data)
        return false;

    uint8_t ip_version = (data[0] >> 4) & 0xF; // first half byte

    // if ipv6 then it's not part of our project
    if (ip_version != 4)
        return false;

    std::memcpy(&src_ip, data + 12, sizeof(uint32_t));
    std::memcpy(&dst_ip, data + 16, sizeof(uint32_t));

    // ntohl == network to host long (big endian to host endianness)
    src_ip = ntohl(src_ip);
    dst_ip = ntohl(dst_ip);

    return true;
}
