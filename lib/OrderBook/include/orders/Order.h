#pragma once

#include "OrderCore.h"

class Order;

class Order : public OrderCore {
private:
    long price_{};
    uint32_t initialQuantity_{};
    uint32_t currentQuantity_{};
    bool isBuy_{};

public:
    Order(const OrderCore &orderCore, long price,
          uint32_t quantity, bool isBuy);

    Order() : OrderCore(-1, "invalid", -1) {}; // FIXME

    [[nodiscard]] inline long Price() const {
        return price_;
    }

    [[nodiscard]] inline uint32_t InitialQuantity() const {
        return initialQuantity_;
    }

    [[nodiscard]] inline uint32_t CurrentQuantity() const {
        return currentQuantity_;
    }

    [[nodiscard]] inline bool IsBuy() const {
        return isBuy_;
    }

    friend class OrderBookEntry;

protected:
    void DecreaseQuantity(uint32_t quantity);
};
