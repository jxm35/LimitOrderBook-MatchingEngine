#pragma once
#include <string>
#include <chrono>

namespace mdfeed
{
    struct PublisherConfig
    {
        std::string multicast_ip = "239.1.1.1";
        uint16_t multicast_port = 9999;
        std::string interface_ip = "127.0.0.1";
        std::chrono::milliseconds heartbeat_interval{1000};
        bool use_busy_wait = false;
        std::chrono::microseconds spin_duration{100};
        size_t ring_buffer_size = 65536;
    };
}

