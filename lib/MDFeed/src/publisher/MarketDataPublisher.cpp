#include "publisher/MarketDataPublisher.h"
#include <print>

namespace mdfeed
{
    MarketDataPublisher::MarketDataPublisher(const PublisherConfig& config) : config_(config)
    {
        ring_buffer_ = std::make_unique<MDRingBuffer>(config_.ring_buffer_size);
    }

    MarketDataPublisher::~MarketDataPublisher()
    {
        stop();
    }

    void MarketDataPublisher::start()
    {
        // Nothing to do
    }

    void MarketDataPublisher::stop()
    {
        // Nothing to do
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
