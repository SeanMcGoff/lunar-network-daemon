# src/netfilter/CMakeLists.txt

add_library(netfilter STATIC
    NetfilterManager.hpp
    NetfilterManager.cpp)

target_link_libraries(netfilter PUBLIC packet)

target_include_directories(netfilter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})