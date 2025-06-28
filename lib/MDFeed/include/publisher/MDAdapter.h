#pragma once

#include "messages/Messages.h"

namespace mdfeed
{
    template <typename PublisherT>
    class MDAdapter
    {
    private:
        PublisherT& publisher_;
        uint32_t instrument_id_;

    public:
        MDAdapter(uint32_t instrument_id, PublisherT& publisher)
            : publisher_(publisher), instrument_id_(instrument_id)
        {
        }

        void notify_price_level_change(uint64_t price, uint64_t new_quantity,
                                       uint64_t old_quantity, bool is_bid)
        {
            Side side = is_bid ? Side::BUY : Side::SELL;
            if (old_quantity == 0 && new_quantity > 0)
            {
                publisher_.publish_price_level_update(instrument_id_, price, new_quantity, side, UpdateAction::NEW);
            }
            else if (old_quantity > 0 && new_quantity > 0)
            {
                publisher_.publish_price_level_update(instrument_id_, price, new_quantity, side, UpdateAction::CHANGE);
            }
            else if (old_quantity > 0 && new_quantity == 0)
            {
                publisher_.publish_price_level_delete(instrument_id_, price, side);
            }
        }

        void notify_trade(uint64_t trade_id, uint64_t price, uint64_t quantity, bool buyer_aggressor)
        {
            Side aggressor_side = buyer_aggressor ? Side::BUY : Side::SELL;
            publisher_.publish_trade(instrument_id_, trade_id, price, quantity, aggressor_side);
        }

        void notify_book_clear(uint32_t reason_code = 0)
        {
            publisher_.publish_book_clear(instrument_id_, reason_code);
        }
    };
}
