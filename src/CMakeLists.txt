# src/CMakeLists.txt

# Find Netfilter Queue library
find_library(NETFILTER_QUEUE_LIBRARY NAMES netfilter_queue)
find_path(NETFILTER_QUEUE_INCLUDE_DIR NAMES libnetfilter_queue/libnetfilter_queue.h)

if(NOT NETFILTER_QUEUE_LIBRARY)
    message(FATAL_ERROR "libnetfilter_queue library not found. Please install libnetfilter-queue-dev package.")
endif()

if(NOT NETFILTER_QUEUE_INCLUDE_DIR)
    message(FATAL_ERROR "libnetfilter_queue headers not found. Please install libnetfilter-queue-dev package.")
endif()

# Find NFNetlink library (dependency of Netfilter Queue)
find_library(NFNETLINK_LIBRARY NAMES nfnetlink)
find_path(NFNETLINK_INCLUDE_DIR NAMES libnfnetlink/libnfnetlink.h)

if(NOT NFNETLINK_LIBRARY)
    message(FATAL_ERROR "libnfnetlink library not found. Please install libnfnetlink-dev package.")
endif()

if(NOT NFNETLINK_INCLUDE_DIR)
    message(FATAL_ERROR "libnfnetlink headers not found. Please install libnfnetlink-dev package.")
endif()

# Add library subdirectories
add_subdirectory(config)
add_subdirectory(packet)
add_subdirectory(netfilter)

# Add the main executable
add_executable(lunar-network-daemon main.cpp)

# Include directories
target_include_directories(lunar-network-daemon
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${NETFILTER_QUEUE_INCLUDE_DIR}
        ${NFNETLINK_INCLUDE_DIR}
)

# Link libraries to the executable
target_link_libraries(lunar-network-daemon
    PRIVATE
        packet
        config
        encap_netfilter
    PRIVATE
        ${NETFILTER_QUEUE_LIBRARY}
        ${NFNETLINK_LIBRARY}
)