#pragma once

#include "DeltaGenerator.h"
#include <memory>

namespace mdfeed
{
    class MDAdapter
    {
    private:
        std::unique_ptr<DeltaGenerator> delta_generator_;
        uint32_t instrument_id_;

    public:
        MDAdapter(uint32_t instrument_id,
                  std::unique_ptr<DeltaGenerator> delta_generator = nullptr);

        void notify_price_level_change(uint64_t price, uint64_t new_quantity,
                                       uint64_t old_quantity, bool is_bid);

        void notify_trade(uint64_t trade_id, uint64_t price, uint64_t quantity, bool buyer_aggressor);

        void notify_book_clear(uint32_t reason_code = 0);

        void set_delta_generator(std::unique_ptr<DeltaGenerator> generator);
    };
}
