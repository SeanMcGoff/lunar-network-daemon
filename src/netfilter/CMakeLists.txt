# src/packet/CMakeLists.txt

add_library(encap_netfilter STATIC
    NetfilterQueue.cpp
    NetfilterQueue.hpp)

target_include_directories(encap_netfilter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(encap_netfilter 
PRIVATE 
    config
    packet)