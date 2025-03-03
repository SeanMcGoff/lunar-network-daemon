// Packet.hpp

#pragma once

#include <cstdint> // uint32_t
#include <cstddef> // size_t
#include <chrono>

// Forward declaration
class PacketClassifier;

class Packet
{
public:
    enum class LinkType
    {
        EARTH_TO_EARTH,
        EARTH_TO_MOON,
        MOON_TO_EARTH,
        MOON_TO_MOON,
        OTHER
    };

    enum class Action {
        ACCEPT,
        DROP,
        MODIFY
    };

    // constructor that takes ownership
    Packet(uint32_t id, uint8_t *data, size_t length, uint32_t mark,
           std::chrono::steady_clock::time_point time_received);

    // constructor that references external data
    Packet(uint32_t id, const uint8_t *data, size_t length, uint32_t mark,
        std::chrono::steady_clock::time_point time_received, bool copy_data);

    // copy constructor and assignment
    Packet(const Packet &other);
    Packet &operator=(const Packet &other);

    // move constructor and assignment
    Packet(Packet &&other) noexcept;
    Packet &operator=(Packet &&other) noexcept;

    ~Packet();


    // modification methods
    bool prepareForModification();

    // Getter methods
    uint32_t getId() const;
    const uint8_t *getData() const;
    uint8_t *getMutableData();
    size_t getLength() const;
    uint32_t getMark() const;
    void setMark(uint32_t new_mark);
    std::chrono::steady_clock::time_point getSendTime() const;
    LinkType getLinkType() const;

private:
    uint32_t id;   // netfilter queue's packet ID is a 32-bit unsigned integer
    uint8_t *data; // netfilter API works with raw packet data as array of bytes
    size_t length;

    // whether we own the memory and should delete[] in the destructor
    bool owns_data;

    LinkType link_type;

    // netfilter mark to classify the packet link type
    uint32_t mark;

    std::chrono::steady_clock::time_point time_received;

    friend class PacketClassifier; // allow classifier to access private data
};

// encapsulated classification into own class
class PacketClassifier
{
public:
    static Packet::LinkType classifyPacket(const uint8_t* data, size_t length);
    
private:
    static bool isRoverIP(uint32_t ip);
    static bool isBaseIP(uint32_t ip);
    static bool extractIPs(const uint8_t* data, size_t length, uint32_t &src_ip, uint32_t &dst_ip);
};
