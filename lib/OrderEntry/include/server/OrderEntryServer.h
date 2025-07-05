#pragma once

#include "utils/OrderBuffer.h"
#include "utils/ServerConfig.h"
#include <atomic>
#include <memory>
#include <thread>

namespace orderentry {
    class OrderEntryServer {
    public:
        explicit OrderEntryServer(uint16_t port = 8080);
        ~OrderEntryServer();

        bool start();
        void stop();
        bool is_running() const { return running_.load(); }

        OrderRingBuffer* get_order_buffer() const
        {
            return order_buffer_.get();
        }

        struct Stats {
            uint64_t orders_received = 0;
            uint64_t connections = 0;
        };

        Stats get_stats() const { return stats_; }

    private:
        void server_loop();
        void handle_client(int client_fd);

        uint16_t port_;
        int server_fd_;
        std::unique_ptr<OrderRingBuffer> order_buffer_;
        std::atomic<bool> running_;
        std::thread server_thread_;
        Stats stats_;
    };
} // namespace orderentry
