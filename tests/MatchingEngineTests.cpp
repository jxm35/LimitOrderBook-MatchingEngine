#include <gtest/gtest.h>
#include <boost/optional/optional_io.hpp>

#include <iostream>
#include <random>

#include "core/OrderBook.h"
#include "publisher/MarketDataPublisher.h"

static OrderBook<mdfeed::NullMarketDataPublisher> createOrderBook() {
    const int SECURITY_ID = 1;
    Security apl("apple", "AAPL", SECURITY_ID);
    mdfeed::NullMarketDataPublisher publisher = mdfeed::NullMarketDataPublisher();
    mdfeed::MDAdapter mdAdapter(SECURITY_ID, publisher);
    return {apl, mdAdapter};
}

TEST(OrderBookTests, CanMatchCrossedSpreadEqualQuantity) {
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

TEST(OrderBookTests, CanMatchCrossedSpreadMoreBids) {
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

TEST(OrderBookTests, CanMatchCrossedSpreadMoreAsks) {
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

TEST(OrderBookTests, DoesntMatchInsufficientBids) {
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

TEST(OrderBookTests, BenchmarkPerformance) {
    std::random_device device_random_;
    std::default_random_engine generator_(device_random_());
    std::normal_distribution<> buyDist(50, 3);
    std::normal_distribution<> sellDist(55, 3);
    std::normal_distribution<> quantityDist(1000, 20);
    std::vector<long> buys;
    std::vector<long> sells;
    std::vector<uint32_t> quantities;
    for (int counter_(0); counter_ < 50001; ++counter_) {
        buys.push_back(buyDist(generator_));
        sells.push_back(sellDist(generator_));
        quantities.push_back(quantityDist(generator_));
    }

    const int SECURITY_ID = 1;
    const std::string USERNAME = "test";
    Security apl("apple", "AAPL", SECURITY_ID);
    auto book = createOrderBook();

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 50000; i++) {
        Order bid(OrderCore(USERNAME, SECURITY_ID), buys[i], quantities[i], true);
        book.AddOrder(bid);
        Order ask(OrderCore(USERNAME, SECURITY_ID), sells[i], quantities[i], false);
        book.AddOrder(ask);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Elapsed Time: " << duration.count() << "ms";

}

TEST(OrderBookTests, TestRandSimluation) {
    const int SECURITY_ID = 1;
    const std::string USERNAME = "test";
    const long MIN_DEVIANCE = 3;

    std::random_device device_random_;
    std::default_random_engine generator_(device_random_());
    std::bernoulli_distribution boolDist(0.5);
    std::normal_distribution<> quantityDist(1000, 20);
    boolDist(generator_);

    Security apl("apple", "AAPL", SECURITY_ID);
    auto book = createOrderBook();

    Order firstBid(OrderCore(USERNAME, SECURITY_ID), 499, quantityDist(generator_), true);
    Order firstAsk(OrderCore(USERNAME, SECURITY_ID), 500, quantityDist(generator_), false);
    book.AddOrder(firstBid);
    book.AddOrder(firstAsk);


    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 50000; i++) {
        long bestBid = book.GetBestBidPrice().value();
        long bestAsk = book.GetBestAskPrice().value();
        double midPrice = (bestAsk + bestBid) / (double) 2;
        // 2 limit orders
        std::normal_distribution<> priceDist(midPrice, 8);
        for (int j = 0; j < 2; j++) {
            double priceDouble = priceDist(generator_);
            long price = round(priceDouble);
            if (midPrice - price < MIN_DEVIANCE && price - midPrice < MIN_DEVIANCE)
                continue;
            if (price > bestAsk) {
                Order ask(OrderCore(USERNAME, SECURITY_ID), price, quantityDist(generator_), false);
                book.AddOrder(ask);
            } else if (price < bestBid) {
                Order bid(OrderCore(USERNAME, SECURITY_ID), price, quantityDist(generator_), true);
                book.AddOrder(bid);
            } else {
                // make a market order
            }
        }
        // market order
        bool isBuy = boolDist(generator_);
        Order order(OrderCore(USERNAME, SECURITY_ID), isBuy ? bestAsk : bestBid, quantityDist(generator_), isBuy);
        book.AddOrder(order);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Elapsed Time: " << duration.count() << "ms";

}