#pragma once

#include <boost/lockfree/spsc_queue.hpp>
#include <cstring>

namespace orderentry {
    struct OrderBuffer {
        static constexpr size_t MAX_MESSAGE_SIZE = 256;
        alignas(8) char data[MAX_MESSAGE_SIZE];
        size_t length;
        int client_fd;

        OrderBuffer(): length(0), client_fd(-1) {}

        template<typename T> OrderBuffer(const T& message, int fd)
            : length(sizeof(T)), client_fd(fd)
        {
            static_assert(sizeof(T) <= MAX_MESSAGE_SIZE,
                          "Message too large for buffer");
            std::memcpy(data, &message, sizeof(T));
        }

        template<typename T> T* as()
        {
            static_assert(sizeof(T) <= MAX_MESSAGE_SIZE,
                          "Message too large for buffer");
            return reinterpret_cast<T*>(data);
        }

        template<typename T> const T* as() const
        {
            static_assert(sizeof(T) <= MAX_MESSAGE_SIZE,
                          "Message too large for buffer");
            return reinterpret_cast<const T*>(data);
        }
    };

    using OrderRingBuffer = boost::lockfree::spsc_queue<OrderBuffer>;
} // namespace orderentry
