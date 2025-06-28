#pragma once

#include <memory>

#include "MarketDataPublisher.h"
#include "messages/Messages.h"

namespace mdfeed {

class DeltaGenerator {
public:
    virtual ~DeltaGenerator() = default;

    virtual void on_price_level_update(uint32_t instrument_id, uint64_t price,
                                       uint64_t quantity, Side side, UpdateAction action) = 0;

    virtual void on_price_level_delete(uint32_t instrument_id, uint64_t price, Side side) = 0;

    virtual void on_trade(uint32_t instrument_id, uint64_t trade_id, uint64_t price,
                          uint64_t quantity, Side aggressor_side) = 0;

    virtual void on_book_clear(uint32_t instrument_id, uint32_t reason_code = 0) = 0;
};

class PublisherDeltaGenerator final : public DeltaGenerator {
public:
    explicit PublisherDeltaGenerator(std::shared_ptr<MarketDataPublisher> publisher);

    void on_price_level_update(uint32_t instrument_id, uint64_t price,
                               uint64_t quantity, Side side, UpdateAction action);

    void on_price_level_delete(uint32_t instrument_id, uint64_t price, Side side);

    void on_trade(uint32_t instrument_id, uint64_t trade_id, uint64_t price,
                  uint64_t quantity, Side aggressor_side);

    void on_book_clear(uint32_t instrument_id, uint32_t reason_code = 0);

private:
    std::shared_ptr<MarketDataPublisher> publisher_;
};

class NullDeltaGenerator final : public DeltaGenerator {
public:
    NullDeltaGenerator() = default;

    void on_price_level_update(uint32_t, uint64_t, uint64_t, Side, UpdateAction) final {}
    void on_price_level_delete(uint32_t, uint64_t, Side) final {}
    void on_trade(uint32_t, uint64_t, uint64_t, uint64_t, Side) final {}
    void on_book_clear(uint32_t, uint32_t) final {}
};

}
