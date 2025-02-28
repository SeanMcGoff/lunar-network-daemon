#include "QueuedPacket.hpp"

#include <cstring> // memcpy

QueuedPacket::QueuedPacket(uint32_t id, uint8_t *data, size_t length, uint32_t mark,
                           clk::time_point send_time)
    : id(id), length(length), mark(mark), send_time(send_time)
{
    // copy data to take ownership
    this->data = new uint8_t[length];
    std::memcpy(this->data, data, length);
}

QueuedPacket::~QueuedPacket()
{
    if (data)
    {
        delete[] data;
        data = nullptr;
    }
}