#include "Packet.hpp"
#include "configs.hpp"

#include <gtest/gtest.h>
#include <netinet/in.h>
#include <vector>

std::vector<uint8_t> make_test_packet(const uint32_t source_ip,
                                      const uint32_t dest_ip) {
  // 0100 (IP version) 0101 (IHL Flag)
  constexpr uint8_t test_packet[] = {0x45, 0x00, 0x00, 0x00, 0x00, 0x00,
                                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  // Create a vector to hold the full packet (header + IPs)
  std::vector<uint8_t> packet(test_packet, test_packet + sizeof(test_packet));

  // Swap endianness
  auto src = ntohl(source_ip);
  auto dst = ntohl(dest_ip);

  // Copy the source and destination IPs into the packet
  packet.insert(packet.end(), reinterpret_cast<uint8_t *>(&src),
                reinterpret_cast<uint8_t *>(&src) + sizeof(src));
  packet.insert(packet.end(), reinterpret_cast<uint8_t *>(&dst),
                reinterpret_cast<uint8_t *>(&dst) + sizeof(dst));

  return packet;
}

TEST(PacketTests, PacketClassInstantiation) {
  uint8_t data = 25;
  Packet p((uint32_t)0, &data, (size_t)sizeof(data), (uint32_t)0,
           std::chrono::steady_clock::now());
  EXPECT_EQ(*p.getData(), data);
}

TEST(PacketTests, EarthToEarthClassification) {
  const auto e2e_packet = make_test_packet(BASE_IP_MIN, BASE_IP_MAX);
  EXPECT_EQ(PacketClassifier::classifyPacket(e2e_packet.data(), 20),
            Packet::LinkType::EARTH_TO_EARTH);
}

TEST(PacketTests, EarthToMoonClassification) {
  const auto e2m_packet = make_test_packet(BASE_IP_MIN, ROVER_IP_MAX);
  EXPECT_EQ(PacketClassifier::classifyPacket(e2m_packet.data(), 20),
            Packet::LinkType::EARTH_TO_MOON);
}

TEST(PacketTests, MoonToEarthClassification) {
  const auto m2e_packet = make_test_packet(ROVER_IP_MIN, BASE_IP_MAX);
  EXPECT_EQ(PacketClassifier::classifyPacket(m2e_packet.data(), 20),
            Packet::LinkType::MOON_TO_EARTH);
}

TEST(PacketTests, MoonToMoonClassification) {
  const auto m2m_packet = make_test_packet(ROVER_IP_MIN, ROVER_IP_MAX);
  EXPECT_EQ(PacketClassifier::classifyPacket(m2m_packet.data(), 20),
            Packet::LinkType::MOON_TO_MOON);
}

TEST(PacketTests, OtherClassification) {
  constexpr uint32_t BAD_IP_1 = (192 << 24 | 168 << 16 | 0 << 8 | 1);
  constexpr uint32_t BAD_IP_2 = (10 << 24 | 0 << 16 | 0 << 8 | 1);

  const auto other_packet = make_test_packet(BAD_IP_1, BAD_IP_2);
  EXPECT_EQ(PacketClassifier::classifyPacket(other_packet.data(), 20),
            Packet::LinkType::OTHER);
}