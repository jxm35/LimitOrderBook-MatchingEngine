#pragma once
#include <chrono>
#include <string>

namespace orderentry {
    struct ServerConfig {
        std::string bind_address = "127.0.0.1";
        uint16_t port = 8080;
        int max_connections = 100;
        size_t ring_buffer_size = 65536;
        std::chrono::milliseconds client_timeout{30000};
        size_t receive_buffer_size = 8192;
        bool enable_logging = true;
        bool log_to_console = true;
        std::string log_file_path;
    };

} // namespace orderentry
