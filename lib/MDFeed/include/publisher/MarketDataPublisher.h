#pragma once

#include <thread>

#include "MulticastPublisher.h"
#include "messages/Messages.h"
#include "utils/RingBuffer.h"
#include "utils/PublisherConfig.h"

namespace mdfeed
{
    class MarketDataPublisher
    {
    private:
        PublisherConfig config_;
        std::unique_ptr<MDRingBuffer> ring_buffer_;
        std::unique_ptr<MulticastPublisher> udp_sender_;

        std::atomic<bool> running_{false};
        std::atomic<uint64_t> sequence_number_{1};
        std::thread publisher_thread_;

        // Statistics
        std::atomic<uint64_t> messages_sent_{0};
        std::atomic<uint64_t> messages_dropped_{0};
        std::atomic<uint64_t> heartbeats_sent_{0};

        void publisher_loop();
        void send_heartbeat();

    public:
        explicit MarketDataPublisher(const PublisherConfig& config = PublisherConfig{});
        ~MarketDataPublisher();

        void start();
        void stop();

        // Enqueue message for publishing
        template <typename T>
        bool enqueue_message(const T& message)
        {
            MessageBuffer buffer(message);
            return ring_buffer_->push(buffer);
        }

        // Helper methods for common message types
        bool publish_price_level_update(uint32_t instrument_id, uint64_t price,
                                        uint64_t quantity, Side side, UpdateAction action);
        bool publish_price_level_delete(uint32_t instrument_id, uint64_t price, Side side);
        bool publish_trade(uint32_t instrument_id, uint64_t trade_id, uint64_t price,
                           uint64_t quantity, Side aggressor_side);
        bool publish_book_clear(uint32_t instrument_id, uint32_t reason_code = 0);

        // Statistics
        uint64_t get_messages_sent() const { return messages_sent_.load(std::memory_order_relaxed); }
        uint64_t get_messages_dropped() const { return messages_dropped_.load(std::memory_order_relaxed); }
        uint64_t get_heartbeats_sent() const { return heartbeats_sent_.load(std::memory_order_relaxed); }
        uint64_t get_current_sequence() const { return sequence_number_.load(std::memory_order_relaxed); }

        // Ring buffer stats
        size_t get_buffer_size() const { return 0; } // Boost doesn't expose size easily
        size_t get_buffer_capacity() const { return config_.ring_buffer_size; }
        bool is_buffer_empty() const { return ring_buffer_->empty(); }
        bool is_running() const { return running_.load(std::memory_order_acquire); }
    };
}
