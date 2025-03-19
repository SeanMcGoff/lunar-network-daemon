# src/CMakeLists.txt
# Add library subdirectories
add_subdirectory(config)
add_subdirectory(packet)
add_subdirectory(netfilter)

# Add the main executable
add_executable(lunar-network-daemon main.cpp)

# Include directories
target_include_directories(lunar-network-daemon
    PRIVATE
    ${NETFILTER_QUEUE_INCLUDE_DIR}
    ${NFNETLINK_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link libraries to the executable
target_link_libraries(lunar-network-daemon
    PRIVATE
    encap_netfilter
    packet
    config
    ${NETFILTER_QUEUE_LIBRARY}
    ${NFNETLINK_LIBRARY}
    spdlog::spdlog
    fmt::fmt
)