#pragma once

#include "ReceiverConfig.h"
#include "messages/Messages.h"
#include <atomic>
#include <functional>
#include <memory>
#include <thread>

namespace mdfeed {
    class MulticastSocket;

    using MessageHandler = std::function<void(const MessageHeader&,
                                              const void* data, size_t length)>;

    class MulticastReceiver {
    public:
        MulticastReceiver();
        explicit MulticastReceiver(const ReceiverConfig& config);
        ~MulticastReceiver();

        MulticastReceiver(const MulticastReceiver&) = delete;
        MulticastReceiver& operator=(const MulticastReceiver&) = delete;

        bool initialize(const ReceiverConfig& config);
        bool start();
        void stop();
        bool is_running() const { return running_.load(); }

        void set_message_handler(MessageHandler handler)
        {
            message_handler_ = std::move(handler);
        }

        struct Stats {
            uint64_t total_messages_received = 0;
            uint64_t total_bytes_received = 0;
            uint64_t sequence_gaps = 0;
            uint64_t invalid_messages = 0;
            std::chrono::steady_clock::time_point start_time;
        };

        Stats get_stats() const { return stats_; }
        void reset_stats();

    private:
        void receiver_loop();
        void process_message(const void* data, size_t length);
        void log_message(const std::string& message) const;
        void print_stats();
        static std::string format_message_debug(const MessageHeader& header,
                                                const void* data);

        ReceiverConfig config_;
        std::unique_ptr<MulticastSocket> socket_;
        std::atomic<bool> running_;
        std::thread receiver_thread_;
        std::thread stats_thread_;

        MessageHandler message_handler_;
        Stats stats_;
        uint64_t last_sequence_number_;
        std::chrono::steady_clock::time_point last_stats_time_;

        std::unique_ptr<std::ofstream> log_file_;
    };
} // namespace mdfeed
