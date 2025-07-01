#include "orders/Order.h"
#include "spdlog/fmt/fmt.h"

Order::Order(const OrderCore &orderCore, long price, uint32_t quantity, bool isBuy)
        : OrderCore(orderCore) {
    price_ = price;
    initialQuantity_ = quantity;
    currentQuantity_ = quantity;
    isBuy_ = isBuy;
}

void Order::DecreaseQuantity(uint32_t quantity) {
    if (quantity > currentQuantity_)
        throw std::invalid_argument(
                fmt::format("Quantity decrease greater than current quantity for OrderId: {}", OrderId()));
    currentQuantity_ -= quantity;

}
