# src/CMakeLists.txt

# Add library subdirectories
add_subdirectory(config)
add_subdirectory(packet)
add_subdirectory(netfilter)

# Add the main executable
add_executable(lunar-network-daemon main.cpp)

# Link static libraries to the executable
target_link_libraries(lunar-network-daemon
    PRIVATE
        packet
        config
        netfilter
)
