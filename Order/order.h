#pragma once

#include "OrderCore.h"
#include "CancelOrder.h"

class Order;
class ModifyOrder;


class Order: public OrderCore {
private:
    long price_;
    uint16_t initialQuantity_;
    uint16_t currentQuantity_;
    bool isBuy_;

public:
    Order(OrderCore orderCore, long price,
          uint32_t quantity, bool isBuy);


    Order(const ModifyOrder& modifyOrder);
    Order(): OrderCore(-1, "invalid", -1) {}; // ToFix

    inline long Price() const {
        return price_;
    }
    inline uint16_t  InitialQuantity() const {
        return initialQuantity_;
    }
    inline uint16_t  CurrentQuantity() const {
        return currentQuantity_;
    }
    inline bool IsBuy() const {
        return isBuy_;
    }

    void IncreaseQuantity(uint16_t quantity);
    void DecreaseQuantity(uint16_t quantity);
};

// Modify Order

class ModifyOrder: public OrderCore {
    long price_;
    uint16_t quantity_;
    bool isBuy_;

public:
    ModifyOrder(OrderCore orderCore, long price, uint16_t quantity, bool isBuy);

    inline long Price() const {
        return price_;
    }
    inline uint16_t Quantity() const {
        return quantity_;
    }
    inline bool IsBuy() const {
        return isBuy_;
    }

    CancelOrder ToCancelOrder();

    Order ToNewOrder();

};


