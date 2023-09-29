#pragma once

#include "OrderCore.h"
#include "CancelOrder.h"

class Order;

class ModifyOrder;


class Order : public OrderCore {
private:
    long price_;
    uint32_t initialQuantity_;
    uint32_t currentQuantity_;
    bool isBuy_;

public:
    Order(OrderCore orderCore, long price,
          uint32_t quantity, bool isBuy);


    Order(const ModifyOrder &modifyOrder);

    Order() : OrderCore(-1, "invalid", -1) {}; // ToFix

    inline long Price() const {
        return price_;
    }

    inline uint32_t InitialQuantity() const {
        return initialQuantity_;
    }

    inline uint32_t CurrentQuantity() const {
        return currentQuantity_;
    }

    inline bool IsBuy() const {
        return isBuy_;
    }

    friend class OrderBookEntry;

protected:
    void DecreaseQuantity(uint32_t quantity);
};

// Modify Order

class ModifyOrder : public OrderCore {
    long price_;
    uint32_t quantity_;
    bool isBuy_;

public:
    ModifyOrder(OrderCore orderCore, long price, uint32_t quantity, bool isBuy);

    inline long Price() const {
        return price_;
    }

    inline uint32_t Quantity() const {
        return quantity_;
    }

    inline bool IsBuy() const {
        return isBuy_;
    }

    CancelOrder ToCancelOrder();

    Order ToNewOrder();

};


