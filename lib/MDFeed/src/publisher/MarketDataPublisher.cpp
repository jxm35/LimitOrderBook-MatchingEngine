#include "publisher/MarketDataPublisher.h"
#include <print>

namespace mdfeed
{
    MarketDataPublisher::MarketDataPublisher(const PublisherConfig& config) : config_(config)
    {
        ring_buffer_ = std::make_unique<MDRingBuffer>(config_.ring_buffer_size);
        udp_sender_ = std::make_unique<MulticastPublisher>();
    }

    MarketDataPublisher::~MarketDataPublisher()
    {
        stop();
    }

    void MarketDataPublisher::start()
    {
        if (running_.exchange(true, std::memory_order_acq_rel))
        {
            return;
        }
        publisher_thread_ = std::thread(&MarketDataPublisher::publisher_loop, this);
    }

    void MarketDataPublisher::stop()
    {
        if (!running_.exchange(false, std::memory_order_acq_rel))
        {
            return;
        }
        if (publisher_thread_.joinable())
        {
            publisher_thread_.join();
        }
    }

    void MarketDataPublisher::publisher_loop()
    {
        auto last_heartbeat = std::chrono::steady_clock::now();
        MessageBuffer msg_buffer;

        std::println("Market data publisher started on {}:{}", config_.multicast_ip, config_.multicast_port);

        while (running_.load(std::memory_order_acquire))
        {
            bool found_message = false;
            while (ring_buffer_->pop(msg_buffer))
            {
                found_message = true;
                if (udp_sender_->send(msg_buffer.data, msg_buffer.length))
                {
                    messages_sent_.fetch_add(1, std::memory_order_relaxed);
                }
                else
                {
                    messages_dropped_.fetch_add(1, std::memory_order_relaxed);
                }
            }

            auto now = std::chrono::steady_clock::now();
            if (now - last_heartbeat >= config_.heartbeat_interval)
            {
                send_heartbeat();
                last_heartbeat = now;
            }

            if (!found_message)
            {
                if (config_.use_busy_wait)
                {
                    auto spin_start = std::chrono::steady_clock::now();
                    while (std::chrono::steady_clock::now() - spin_start < config_.spin_duration)
                    {
                        std::this_thread::yield();
                    }
                }
                else
                {
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                }
            }
        }

        std::println("Market data publisher stopped. Messages sent: {}, dropped: {}", messages_sent_.load(),
                     messages_dropped_.load());
    }

    void MarketDataPublisher::send_heartbeat()
    {
        HeartbeatMessage heartbeat;
        message_utils::init_header(heartbeat, MessageType::HEARTBEAT,
                                   sequence_number_.fetch_add(1, std::memory_order_acq_rel), 0);

        if (udp_sender_->send_message(heartbeat))
        {
            heartbeats_sent_.fetch_add(1, std::memory_order_relaxed);
        }
    }

    bool MarketDataPublisher::publish_price_level_update(uint32_t instrument_id, uint64_t price,
                                                         uint64_t quantity, Side side, UpdateAction action)
    {
        PriceLevelUpdateMessage msg;
        message_utils::init_header(msg, MessageType::PRICE_LEVEL_UPDATE,
                                   sequence_number_.fetch_add(1, std::memory_order_acq_rel),
                                   instrument_id);
        msg.price = price;
        msg.quantity = quantity;
        msg.side = side;
        msg.action = action;
        std::memset(msg.reserved, 0, sizeof(msg.reserved));

        return enqueue_message(msg);
    }

    bool MarketDataPublisher::publish_price_level_delete(uint32_t instrument_id, uint64_t price, Side side)
    {
        PriceLevelDeleteMessage msg;
        message_utils::init_header(msg, MessageType::PRICE_LEVEL_DELETE,
                                   sequence_number_.fetch_add(1, std::memory_order_acq_rel),
                                   instrument_id);
        msg.price = price;
        msg.side = side;
        std::memset(msg.reserved, 0, sizeof(msg.reserved));

        return enqueue_message(msg);
    }

    bool MarketDataPublisher::publish_trade(uint32_t instrument_id, uint64_t trade_id, uint64_t price,
                                            uint64_t quantity, Side aggressor_side)
    {
        TradeMessage msg;
        message_utils::init_header(msg, MessageType::TRADE,
                                   sequence_number_.fetch_add(1, std::memory_order_acq_rel),
                                   instrument_id);
        msg.trade_id = trade_id;
        msg.price = price;
        msg.quantity = quantity;
        msg.aggressor_side = aggressor_side;
        std::memset(msg.reserved, 0, sizeof(msg.reserved));

        return enqueue_message(msg);
    }

    bool MarketDataPublisher::publish_book_clear(uint32_t instrument_id, uint32_t reason_code)
    {
        BookClearMessage msg;
        message_utils::init_header(msg, MessageType::BOOK_CLEAR,
                                   sequence_number_.fetch_add(1, std::memory_order_acq_rel),
                                   instrument_id);
        msg.reason_code = reason_code;
        std::memset(msg.reserved, 0, sizeof(msg.reserved));

        return enqueue_message(msg);
    }
}
