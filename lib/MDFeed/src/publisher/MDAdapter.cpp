#include "publisher/MDAdapter.h"

#include "publisher/DeltaGenerator.h"

namespace mdfeed
{
    MDAdapter::MDAdapter(uint32_t instrument_id,
                         std::unique_ptr<DeltaGenerator> delta_generator)
        : delta_generator_(std::move(delta_generator)), instrument_id_(instrument_id)
    {
        if (!delta_generator_)
        {
            delta_generator_ = std::make_unique<NullDeltaGenerator>();
        }
    }

    void MDAdapter::notify_price_level_change(uint64_t price, uint64_t new_quantity,
                                              uint64_t old_quantity, bool is_bid)
    {
        Side side = is_bid ? Side::BUY : Side::SELL;
        if (old_quantity == 0 && new_quantity > 0)
        {
            delta_generator_->on_price_level_update(instrument_id_, price, new_quantity, side, UpdateAction::NEW);
        }
        else if (old_quantity > 0 && new_quantity > 0)
        {
            delta_generator_->on_price_level_update(instrument_id_, price, new_quantity, side, UpdateAction::CHANGE);
        }
        else if (old_quantity > 0 && new_quantity == 0)
        {
            delta_generator_->on_price_level_delete(instrument_id_, price, side);
        }
    }

    void MDAdapter::notify_trade(uint64_t trade_id, uint64_t price, uint64_t quantity, bool buyer_aggressor)
    {
        Side aggressor_side = buyer_aggressor ? Side::BUY : Side::SELL;
        delta_generator_->on_trade(instrument_id_, trade_id, price, quantity, aggressor_side);
    }

    void MDAdapter::notify_book_clear(uint32_t reason_code)
    {
        delta_generator_->on_book_clear(instrument_id_, reason_code);
    }

    void MDAdapter::set_delta_generator(std::unique_ptr<DeltaGenerator> generator)
    {
        delta_generator_ = std::move(generator);
        if (!delta_generator_)
        {
            delta_generator_ = std::make_unique<NullDeltaGenerator>();
        }
    }
}
