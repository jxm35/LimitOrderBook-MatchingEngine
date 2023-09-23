#include <gtest/gtest.h>
#include "OrderBook.h"

TEST(OrderBookTests, OrderBookInitialisesCorrectly) {
    Security apl("apple", "AAPL", 1);
    OrderBook book(apl);
    EXPECT_EQ(book.Count(), 0);
    EXPECT_EQ(book.GetAskOrders().size(), 0);
    EXPECT_EQ(book.GetBidOrders().size(), 0);
}

TEST(OrderBookTests, CanAddBids) {
    const int SECURITY_ID = 1;
    const std::string USERNAME = "test";
    Security apl("apple", "AAPL", SECURITY_ID);
    OrderBook book(apl);
    Order order1(OrderCore(1, USERNAME, SECURITY_ID), 50, 20, true);
    book.AddOrder(order1);
    EXPECT_EQ(book.Count(), 1);
    std::list<OrderBookEntry> bids = book.GetBidOrders();
    EXPECT_EQ(bids.begin()->Limit()->Price(), 50);
    EXPECT_EQ(bids.begin()->Limit()->IsEmpty(), false);
//    EXPECT_EQ(bids.begin()->Limit()->GetOrderCount(), 1);
    EXPECT_EQ(bids.begin()->Limit()->GetOrderQuantity(), 20);
    EXPECT_EQ(bids.begin()->CurrentOrder().orderId_, order1.orderId_);
}