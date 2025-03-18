// Packet.hpp

// ---- Packet Usage ---- //

// Packet is a class that lays the foundation for efficient handling of packet
// data while staying compatible with the netfilter C API and raw pointers.

// It allows the user to access members like LinkType and netfilter mark which
// are useful for processing downstream.

// It provides utilities for constructing, modifying, copying, and classifying
// packets

// There are two constructors, one that always copies data
// Example:
// Packet pkt(id, data_ptr, data_length, mark,
// std::chrono::steady_clock::now()); NB: caller must handle std::bad_alloc
// exception

// And a second that optionally references data without copying
// This is more efficient but the user should ensure the data will outlive the
// packet instance Example (true to copy, false to not copy): Packet pkt(id,
// data_ptr, data_length, mark, std::chrono::steady_clock::now(), false); NB:
// caller must handle std::bad_alloc exception

// The packet automatically classifies traffic based on IP addresses in the
// constructor. Example: Packet::LinkType link = pkt.getLinkType(); // returns
// EARTH_TO_MOON, MOON_TO_MOON, etc.

// For read-only access to packet data, use getData()
// Example:
// const uint8_t *data = pkt.getData();
// size_t length = pkt.getLength;

// For modifying packet data, use getMutableData() which ensures data is copied
// if needed Example: uint8_t *mutable_data = pkt.getMutableData(); if
// (mutable_data) {
//      mutable_data[10] = 0x42; // you can modify some byte
// }
// NB: caller must handle std::bad_alloc exception

// Or you can prepare for modification explicitly
// Example:
// if (pkt.prepareForModification())
//      do_something;
// NB: caller must handle std::bad_alloc exception

// Get or set netfilter mark
// Example:
// uint32_t mark = pkt.getMark();
// pkt.setMark(new_mark);

// The class tracks data ownership when moving or copying
// The copy constructor will only modify the data in pkt2 if the copy is
// successful Example: Packet pkt2 = pkt; // maks a deep copy only if pkt owns
// its data NB: caller must handle std::bad_alloc exception

// Ownership can be transferred easily using std::move
// This leaves the original packet in a valid but empty state
// Example:
// Packet pkt3 = std::move(pkt); // transfers ownership efficiently

// When a packet is destroyed, it will free memory only if it owns the data

#pragma once

#include <chrono>  // std::chrono::stead_clock
#include <cstddef> // size_t
#include <cstdint> // uint32_t
#include <string>

// Forward declaration
class PacketClassifier;

class Packet {
public:
  enum class LinkType {
    EARTH_TO_EARTH,
    EARTH_TO_MOON,
    MOON_TO_EARTH,
    MOON_TO_MOON,
    OTHER
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

  bool prepareForModification();

  // Getter methods
  uint32_t getId() const;
  const uint8_t *getData() const;
  uint8_t *getMutableData();
  size_t getLength() const;
  uint32_t getMark() const;
  void setMark(uint32_t new_mark);
  std::chrono::steady_clock::time_point getTimeReceived() const;
  LinkType getLinkType() const;
  const std::string getLinkTypeName(Packet::LinkType type);

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
class PacketClassifier {
public:
  static Packet::LinkType classifyPacket(const uint8_t *data, size_t length);

private:
  static bool isRoverIP(uint32_t ip);
  static bool isBaseIP(uint32_t ip);
  static bool extractIPs(const uint8_t *data, size_t length, uint32_t &src_ip,
                         uint32_t &dst_ip);
};
