#include <gtest/gtest.h>
#include "order.h"
#include "Security.h"
#include "OrderBookEntry.h"

TEST(OrderBookTests, OrderBookEntry) {
    Security sec("name", "code", 1);
    Order order(OrderCore("username", 1), 10, 5, true);
    Limit *lim = new Limit(10);
    OrderBookEntry *obe = new OrderBookEntry(lim, order);
    EXPECT_EQ(obe->CurrentOrder().CurrentQuantity(), 5);
//    obe->CurrentOrder().DecreaseQuantity(2);
//    order.DecreaseQuantity(2);
    obe->DecreaseQuantity(2);
    EXPECT_EQ(obe->CurrentOrder().CurrentQuantity(), 5 - 2);
}