#include "publisher/DeltaGenerator.h"
#include "publisher/MDAdapter.h"

namespace mdfeed
{
    PublisherDeltaGenerator::PublisherDeltaGenerator(std::shared_ptr<MarketDataPublisher> publisher)
        : publisher_(std::move(publisher))
    {
    }

    void PublisherDeltaGenerator::on_price_level_update(uint32_t instrument_id, uint64_t price,
                                                        uint64_t quantity, Side side, UpdateAction action)
    {
        if (publisher_)
        {
            publisher_->publish_price_level_update(instrument_id, price, quantity, side, action);
        }
    }

    void PublisherDeltaGenerator::on_price_level_delete(uint32_t instrument_id, uint64_t price, Side side)
    {
        if (publisher_)
        {
            publisher_->publish_price_level_delete(instrument_id, price, side);
        }
    }

    void PublisherDeltaGenerator::on_trade(uint32_t instrument_id, uint64_t trade_id, uint64_t price,
                                           uint64_t quantity, Side aggressor_side)
    {
        if (publisher_)
        {
            publisher_->publish_trade(instrument_id, trade_id, price, quantity, aggressor_side);
        }
    }

    void PublisherDeltaGenerator::on_book_clear(uint32_t instrument_id, uint32_t reason_code)
    {
        if (publisher_)
        {
            publisher_->publish_book_clear(instrument_id, reason_code);
        }
    }
}
