#include "core/OrderBook.h"
#include "publisher/MarketDataPublisher.h"
#include <boost/optional/optional_io.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <random>

static OrderBook<mdfeed::NullMarketDataPublisher> createOrderBook()
{
    const int SECURITY_ID = 1;
    Security apl("apple", "AAPL", SECURITY_ID);
    mdfeed::NullMarketDataPublisher publisher
            = mdfeed::NullMarketDataPublisher();
    mdfeed::MDAdapter mdAdapter(SECURITY_ID, publisher);
    return {apl, mdAdapter};
}

TEST(OrderBookTests, CanMatchCrossedSpreadEqualQuantity)
{
    const int SECURITY_ID = 1;
    const std::string USERNAME = "test";
    Security apl("apple", "AAPL", SECURITY_ID);
    auto book = createOrderBook();
    Order bid(OrderCore(USERNAME, SECURITY_ID), 51, 20, true);
    Order ask(OrderCore(USERNAME, SECURITY_ID), 49, 20, false);
    book.AddOrder(bid);
    book.AddOrder(ask);
    EXPECT_FALSE(book.ContainsOrder(bid.OrderId()));
    EXPECT_FALSE(book.ContainsOrder(ask.OrderId()));
    EXPECT_EQ(book.Count(), 0);
    EXPECT_EQ(book.GetBestBidLimit(), boost::none);
    EXPECT_EQ(book.GetBestAskLimit(), boost::none);
    EXPECT_EQ(book.GetSpread().Spread(), boost::none);
}

TEST(OrderBookTests, CanMatchCrossedSpreadMoreBids)
{
    const int SECURITY_ID = 1;
    const std::string USERNAME = "test";
    Security apl("apple", "AAPL", SECURITY_ID);
    auto book = createOrderBook();
    Order bid(OrderCore(USERNAME, SECURITY_ID), 51, 20, true);
    Order ask(OrderCore(USERNAME, SECURITY_ID), 49, 15, false);
    book.AddOrder(bid);
    book.AddOrder(ask);
    EXPECT_TRUE(book.ContainsOrder(bid.OrderId()));
    EXPECT_FALSE(book.ContainsOrder(ask.OrderId()));
    EXPECT_EQ(book.Count(), 1);
    EXPECT_EQ(book.GetBestBidLimit().value()->GetOrderQuantity(), 20 - 15);
    EXPECT_EQ(book.GetBestAskLimit(), boost::none);
    EXPECT_EQ(book.GetSpread().Spread(), boost::none);
}

TEST(OrderBookTests, CanMatchCrossedSpreadMoreAsks)
{
    const int SECURITY_ID = 1;
    const std::string USERNAME = "test";
    Security apl("apple", "AAPL", SECURITY_ID);
    auto book = createOrderBook();
    Order bid(OrderCore(USERNAME, SECURITY_ID), 51, 15, true);
    Order ask(OrderCore(USERNAME, SECURITY_ID), 49, 20, false);
    book.AddOrder(bid);
    book.AddOrder(ask);
    EXPECT_FALSE(book.ContainsOrder(bid.OrderId()));
    EXPECT_TRUE(book.ContainsOrder(ask.OrderId()));
    EXPECT_EQ(book.Count(), 1);
    EXPECT_EQ(book.GetBestBidLimit(), boost::none);
    EXPECT_EQ(book.GetBestAskLimit().value()->GetOrderQuantity(), 20 - 15);
    EXPECT_EQ(book.GetSpread().Spread(), boost::none);
}

TEST(OrderBookTests, DoesntMatchInsufficientBids)
{
    const int SECURITY_ID = 1;
    const std::string USERNAME = "test";
    Security apl("apple", "AAPL", SECURITY_ID);
    auto book = createOrderBook();
    Order bid(OrderCore(USERNAME, SECURITY_ID), 49, 15, true);
    Order ask(OrderCore(USERNAME, SECURITY_ID), 51, 20, false);
    book.AddOrder(bid);
    book.AddOrder(ask);
    EXPECT_TRUE(book.ContainsOrder(bid.OrderId()));
    EXPECT_TRUE(book.ContainsOrder(ask.OrderId()));
    EXPECT_EQ(book.Count(), 2);
    EXPECT_EQ(book.GetBestBidLimit().value()->GetOrderQuantity(), 15);
    EXPECT_EQ(book.GetBestAskLimit().value()->GetOrderQuantity(), 20);
    EXPECT_EQ(book.GetSpread().Spread().value(), 51 - 49);
}
