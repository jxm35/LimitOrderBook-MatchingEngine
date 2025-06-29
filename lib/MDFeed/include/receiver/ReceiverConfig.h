#pragma once
#include <string>
#include <chrono>

namespace mdfeed
{
    struct ReceiverConfig
    {
        std::string multicast_ip = "239.1.1.1";
        uint16_t multicast_port = 9999;
        std::string interface_ip = "127.0.0.1";
        size_t receive_buffer_size = 1024 * 1024;
        bool enable_logging = true;
        bool log_to_console = true;
        std::string log_file_path;
        bool validate_sequence_numbers = true;
        std::chrono::milliseconds stats_interval{5000};
    };
}
