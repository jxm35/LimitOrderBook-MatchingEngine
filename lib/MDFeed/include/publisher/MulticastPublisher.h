#pragma once
#include <cstddef>

namespace mdfeed
{
    class MulticastPublisher
    {
    public:
        MulticastPublisher();
        ~MulticastPublisher();

        MulticastPublisher(const MulticastPublisher&) = delete;
        MulticastPublisher& operator=(const MulticastPublisher&) = delete;

        bool send(const void* data, std::size_t length);

        template <typename T>
        bool send_message(const T& message)
        {
            return send(&message, sizeof(T));
        }
    };
}
