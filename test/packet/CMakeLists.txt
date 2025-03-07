enable_testing()

add_executable(
    packet_test
    PacketTest.cpp
)
target_link_libraries(
    packet_test
    config
    packet
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(packet_test)