#pragma once

#include <boost/lockfree/spsc_queue.hpp>
#include <cstring>

namespace mdfeed {

    struct MessageBuffer {
        static constexpr size_t MAX_MESSAGE_SIZE = 256;
        alignas(8) char data[MAX_MESSAGE_SIZE];
        size_t length;

        MessageBuffer() : length(0) {}

        template<typename T>
        MessageBuffer(const T& message) : length(sizeof(T)) {
            static_assert(sizeof(T) <= MAX_MESSAGE_SIZE, "Message too large for buffer");
            std::memcpy(data, &message, sizeof(T));
        }

        template<typename T>
        T* as() {
            static_assert(sizeof(T) <= MAX_MESSAGE_SIZE, "Message too large for buffer");
            return reinterpret_cast<T*>(data);
        }

        template<typename T>
        const T* as() const {
            static_assert(sizeof(T) <= MAX_MESSAGE_SIZE, "Message too large for buffer");
            return reinterpret_cast<const T*>(data);
        }
    };

    using MDRingBuffer = boost::lockfree::spsc_queue<MessageBuffer>;
}
