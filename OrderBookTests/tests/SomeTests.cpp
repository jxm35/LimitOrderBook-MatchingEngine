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
    Order order1(OrderCore(USERNAME, SECURITY_ID), 50, 20, true);
    book.AddOrder(order1);
    EXPECT_EQ(book.Count(), 1);
    EXPECT_TRUE(book.ContainsOrder(order1.OrderId()));
    std::list<OrderBookEntry> bids = book.GetBidOrders();
    EXPECT_EQ(bids.begin()->Limit()->Price(), 50);
    EXPECT_EQ(bids.begin()->Limit()->IsEmpty(), false);
    EXPECT_EQ(bids.begin()->Limit()->GetOrderCount(), 1);
    EXPECT_EQ(bids.begin()->Limit()->GetOrderQuantity(), 20);
    EXPECT_EQ(bids.begin()->CurrentOrder().OrderId(), order1.OrderId());
}

TEST(OrderBookTests, CanAddAsks) {
    const int SECURITY_ID = 1;
    const std::string USERNAME = "test";
    Security apl("apple", "AAPL", SECURITY_ID);
    OrderBook book(apl);
    Order order1(OrderCore(USERNAME, SECURITY_ID), 45, 15, false);
    book.AddOrder(order1);
    EXPECT_EQ(book.Count(), 1);
    EXPECT_TRUE(book.ContainsOrder(order1.OrderId()));
    std::list<OrderBookEntry> bids = book.GetAskOrders();
    EXPECT_EQ(bids.begin()->Limit()->Price(), 45);
    EXPECT_EQ(bids.begin()->Limit()->IsEmpty(), false);
    EXPECT_EQ(bids.begin()->Limit()->GetOrderCount(), 1);
    EXPECT_EQ(bids.begin()->Limit()->GetOrderQuantity(), 15);
    EXPECT_EQ(bids.begin()->CurrentOrder().OrderId(), order1.OrderId());
}

TEST(OrderBookTests, CanAddBidsSameLevel) {
    const int SECURITY_ID = 1;
    const std::string USERNAME = "test";
    Security apl("apple", "AAPL", SECURITY_ID);
    OrderBook book(apl);
    Order order1(OrderCore(USERNAME, SECURITY_ID), 45, 3, true);
    Order order2(OrderCore(USERNAME, SECURITY_ID), 45, 5, true);
    book.AddOrder(order1);
    book.AddOrder(order2);
    EXPECT_EQ(book.Count(), 2);
    std::list<OrderBookEntry> bids = book.GetBidOrders();
    EXPECT_EQ(bids.begin()->Limit()->Price(), 45);
    EXPECT_EQ(bids.begin()->Limit()->IsEmpty(), false);
    EXPECT_EQ(bids.begin()->Limit()->GetOrderCount(), 2);
    EXPECT_EQ(bids.begin()->Limit()->GetOrderQuantity(), 8);
    EXPECT_EQ(bids.begin()->Limit()->head_->CurrentOrder().OrderId(), order1.OrderId());
    EXPECT_EQ(bids.begin()->Limit()->head_->next->CurrentOrder().OrderId(), order2.OrderId());
}

TEST(OrderBookTests, CanGetBidAskSpread) {
    const int SECURITY_ID = 1;
    const std::string USERNAME = "test";
    Security apl("apple", "AAPL", SECURITY_ID);
    OrderBook book(apl);
    Order sellOrder1(OrderCore(USERNAME, SECURITY_ID), 50, 5, false);
    Order sellOrder2(OrderCore(USERNAME, SECURITY_ID), 51, 20, false);
    Order buyOrder1(OrderCore(USERNAME, SECURITY_ID), 48, 15, true);
    Order buyOrder2(OrderCore(USERNAME, SECURITY_ID), 47, 10, true);
    book.AddOrder(buyOrder1);
    book.AddOrder(buyOrder2);
    book.AddOrder(sellOrder1);
    book.AddOrder(sellOrder2);
    EXPECT_EQ(book.GetSpread().Spread().value(), 50 - 48);
    EXPECT_EQ(book.Count(), 4);
}

TEST(OrderBookTests, CanCancelBids) {
    const int SECURITY_ID = 1;
    const std::string USERNAME = "test";
    Security apl("apple", "AAPL", SECURITY_ID);
    OrderBook book(apl);
    Order order1(OrderCore(USERNAME, SECURITY_ID), 50, 20, true);
    book.AddOrder(order1);
    CancelOrder cancelOrder(OrderCore(order1.OrderId(), USERNAME, SECURITY_ID));
    book.RemoveOrder(cancelOrder);
    EXPECT_EQ(book.Count(), 0);
    std::list<OrderBookEntry> bids = book.GetBidOrders();
    EXPECT_EQ(bids.size(), 0);
}

TEST(OrderBookTests, CanModifyBids) {
    const int SECURITY_ID = 1;
    const std::string USERNAME = "test";
    Security apl("apple", "AAPL", SECURITY_ID);
    OrderBook book(apl);
    Order order1(OrderCore(USERNAME, SECURITY_ID), 50, 20, true);
    book.AddOrder(order1);
    ModifyOrder modifyOrder(OrderCore(order1.OrderId(), USERNAME, SECURITY_ID), 50, 15, true);
    book.ChangeOrder(modifyOrder);
    EXPECT_EQ(book.Count(), 1);
    std::list<OrderBookEntry> bids = book.GetBidOrders();
    EXPECT_EQ(bids.begin()->Limit()->Price(), 50);
    EXPECT_EQ(bids.begin()->Limit()->IsEmpty(), false);
    EXPECT_EQ(bids.begin()->Limit()->GetOrderCount(), 1);
    EXPECT_EQ(bids.begin()->Limit()->GetOrderQuantity(), 15);
    EXPECT_EQ(bids.begin()->CurrentOrder().OrderId(), order1.OrderId());
}