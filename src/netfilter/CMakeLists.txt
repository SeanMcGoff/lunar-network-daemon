# src/packet/CMakeLists.txt

add_library(encap_netfilter STATIC
    NetfilterQueue.cpp
    NetfilterQueue.hpp)

target_include_directories(encap_netfilter
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${NETFILTER_QUEUE_INCLUDE_DIR}
        ${NFNETLINK_INCLUDE_DIR}
)


target_link_libraries(encap_netfilter
    PUBLIC
        packet
        config
    PRIVATE
        ${NETFILTER_QUEUE_LIBRARY}
        ${NFNETLINK_LIBRARY}
)