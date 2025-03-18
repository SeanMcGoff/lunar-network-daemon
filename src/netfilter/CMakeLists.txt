# src/packet/CMakeLists.txt

add_library(encap_netfilter STATIC
    NetfilterQueue.cpp
    NetfilterQueue.hpp)

target_include_directories(packet PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})