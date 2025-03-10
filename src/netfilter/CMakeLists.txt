# src/netfilter/CMakeLists.txt

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

add_library(netfilter STATIC
    NetfilterManager.hpp
    NetfilterManager.cpp)

# Include directories
target_include_directories(netfilter
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${NETFILTER_QUEUE_INCLUDE_DIR}
        ${NFNETLINK_INCLUDE_DIR}
)

# Link dependencies
target_link_libraries(netfilter
    PUBLIC
        packet
        config
    PRIVATE
        ${NETFILTER_QUEUE_LIBRARY}
        ${NFNETLINK_LIBRARY}
)