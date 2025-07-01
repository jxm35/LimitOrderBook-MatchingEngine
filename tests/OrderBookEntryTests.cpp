#include <gtest/gtest.h>
#include "orders/Order.h"
#include "securities/Security.h"
#include "entries/OrderBookEntry.h"

TEST(OrderBookTests, OrderBookEntry) {
    Security sec("name", "code", 1);
    Order order(OrderCore("username", 1), 10, 5, true);
    auto lim = std::make_shared<Limit>(10);
    auto obe = new OrderBookEntry(lim, order);
    EXPECT_EQ(obe->CurrentOrder().CurrentQuantity(), 5);
//    obe->CurrentOrder().DecreaseQuantity(2);
//    order.DecreaseQuantity(2);
    obe->DecreaseQuantity(2);
    EXPECT_EQ(obe->CurrentOrder().CurrentQuantity(), 5 - 2);
}