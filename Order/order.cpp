#include "order.h"
#include "fmt/core.h"

Order::Order(OrderCore orderCore, long price, uint32_t quantity, bool isBuy)
        : OrderCore(orderCore) {
    price_ = price;
    initialQuantity_ = quantity;
    currentQuantity_ = quantity;
    isBuy_ = isBuy;
}

Order::Order(const ModifyOrder &modifyOrder) : OrderCore(modifyOrder) {
    price_ = modifyOrder.Price();
    initialQuantity_ = modifyOrder.Quantity();
    currentQuantity_ = modifyOrder.Quantity();
    isBuy_ = modifyOrder.IsBuy();
}

void Order::DecreaseQuantity(uint32_t quantity) {
    if (quantity > currentQuantity_)
        throw std::invalid_argument(
                fmt::format("Quantity decrease greater than current quantity for OrderId: {}", OrderId()));
    currentQuantity_ -= quantity;

}


// Modify Order

ModifyOrder::ModifyOrder(OrderCore orderCore, long price, uint32_t quantity, bool isBuy)
        : OrderCore(orderCore) {
    price_ = price;
    quantity_ = quantity;
    isBuy_ = isBuy;
}

CancelOrder ModifyOrder::ToCancelOrder() {
    return CancelOrder(*this);
}

Order ModifyOrder::ToNewOrder() {
    return Order(*this);
}
