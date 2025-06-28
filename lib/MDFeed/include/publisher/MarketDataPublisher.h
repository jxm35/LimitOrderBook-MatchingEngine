#pragma once

#include "messages/Messages.h"
#include "utils/RingBuffer.h"
#include "utils/PublisherConfig.h"

namespace mdfeed
{
    template <typename Derived>
    class MarketDataPublisherBase
    {
    public:
        void start() { static_cast<Derived*>(this)->start(); }
        void stop() { static_cast<Derived*>(this)->stop(); }

        bool publish_price_level_update(uint32_t instrument_id, uint64_t price,
                                        uint64_t quantity, Side side, UpdateAction action)
        {
            return static_cast<Derived*>(this)->
                publish_price_level_update(instrument_id, price, quantity, side, action);
        }

        bool publish_price_level_delete(uint32_t instrument_id, uint64_t price, Side side)
        {
            return static_cast<Derived*>(this)->publish_price_level_delete(instrument_id, price, side);
        }

        bool publish_trade(uint32_t instrument_id, uint64_t trade_id, uint64_t price,
                           uint64_t quantity, Side aggressor_side)
        {
            return static_cast<Derived*>(this)->publish_trade(instrument_id, trade_id, price, quantity, aggressor_side);
        }

        bool publish_book_clear(uint32_t instrument_id, uint32_t reason_code = 0)
        {
            return static_cast<Derived*>(this)->publish_book_clear(instrument_id, reason_code);
        }
    };

    class MarketDataPublisher : public MarketDataPublisherBase<MarketDataPublisher>
    {
    private:
        PublisherConfig config_;
        std::unique_ptr<MDRingBuffer> ring_buffer_;
        std::atomic<uint64_t> sequence_number_{1};

        template <typename T>
        bool enqueue_message(const T& message)
        {
            MessageBuffer buffer(message);
            return ring_buffer_->push(buffer);
        }

    public:
        explicit MarketDataPublisher(const PublisherConfig& config = PublisherConfig{});
        ~MarketDataPublisher();

        void start();
        void stop();

        bool publish_price_level_update(uint32_t, uint64_t, uint64_t, Side, UpdateAction);
        bool publish_price_level_delete(uint32_t, uint64_t, Side);
        bool publish_trade(uint32_t, uint64_t, uint64_t, uint64_t, Side);
        bool publish_book_clear(uint32_t, uint32_t);
    };

    class NullMarketDataPublisher : public MarketDataPublisherBase<NullMarketDataPublisher>
    {
    public:
        void start()
        {
        }

        void stop()
        {
        }

        bool publish_price_level_update(uint32_t, uint64_t, uint64_t, Side, UpdateAction) { return true; }
        bool publish_price_level_delete(uint32_t, uint64_t, Side) { return true; }
        bool publish_trade(uint32_t, uint64_t, uint64_t, uint64_t, Side) { return true; }
        bool publish_book_clear(uint32_t, uint32_t) { return true; }
    };
}
