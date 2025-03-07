# src/packet/CMakeLists.txt

add_library(packet STATIC
    Packet.cpp
    Packet.hpp)

target_link_libraries(packet PUBLIC config)

target_include_directories(packet PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})