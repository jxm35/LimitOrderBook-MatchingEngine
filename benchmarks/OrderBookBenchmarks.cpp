#include <benchmark/benchmark.h>
#include <random>
#include "securities/Security.h"
#include "core/OrderBook.h"
#include "publisher/MarketDataPublisher.h"
#include "spdlog/spdlog.h"

static OrderBook<mdfeed::NullMarketDataPublisher> createOrderBook() {
    const int SECURITY_ID = 1;
    Security apl("apple", "AAPL", SECURITY_ID);
    mdfeed::NullMarketDataPublisher publisher = mdfeed::NullMarketDataPublisher();
    mdfeed::MDAdapter mdAdapter(SECURITY_ID, publisher);
    return {apl, mdAdapter};
}

// static OrderBook<mdfeed::MarketDataPublisher> createOrderBook()
// {
//     const int SECURITY_ID = 1;
//     Security apl("apple", "AAPL", SECURITY_ID);
//     mdfeed::MarketDataPublisher publisher = mdfeed::MarketDataPublisher();
//     mdfeed::MDAdapter mdAdapter(SECURITY_ID, publisher);
//     return {apl, mdAdapter};
// }

static void BM_PlaceMarketOrder(benchmark::State &state) {
    spdlog::set_level(spdlog::level::err);
    const std::string USERNAME = "test";
    uint64_t i = 0;
    for (auto _: state) {
        auto book = createOrderBook();
        book.AddOrder(Order(OrderCore(USERNAME, 1), 500, 100, true));
        auto start = std::chrono::high_resolution_clock::now();
        book.PlaceMarketSellOrder(50);
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds =
                std::chrono::duration_cast<std::chrono::duration<double>>(
                        end - start);
        state.SetIterationTime(elapsed_seconds.count());
        i++;
    }
    state.SetItemsProcessed(i);
}

static void BM_PlaceMarketOrderAcross3Bids(benchmark::State &state) {
    spdlog::set_level(spdlog::level::err);
    const int SECURITY_ID = 1;
    const std::string USERNAME = "test";
    Security apl("apple", "AAPL", SECURITY_ID);
    uint64_t i = 0;
    for (auto _: state) {
        auto book = createOrderBook();
        book.AddOrder(Order(OrderCore(USERNAME, 1), 500, 100, true));
        book.AddOrder(Order(OrderCore(USERNAME, 1), 500, 100, true));
        book.AddOrder(Order(OrderCore(USERNAME, 1), 500, 100, true));
        auto start = std::chrono::high_resolution_clock::now();
        book.PlaceMarketSellOrder(125);
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds =
                std::chrono::duration_cast<std::chrono::duration<double>>(
                        end - start);
        state.SetIterationTime(elapsed_seconds.count());
        i++;
    }
    state.SetItemsProcessed(i);
}

static void BM_Get_Order(benchmark::State &state) {
    spdlog::set_level(spdlog::level::err);
    const int SECURITY_ID = 1;
    const std::string USERNAME = "test";
    Security apl("apple", "AAPL", SECURITY_ID);
    uint64_t i = 0;
    for (auto _: state) {
        auto book = createOrderBook();
        Order bid(OrderCore(USERNAME, 1), 500, 100, true);
        book.AddOrder(bid);
        auto start = std::chrono::high_resolution_clock::now();
        book.ContainsOrder(bid.OrderId());
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds =
                std::chrono::duration_cast<std::chrono::duration<double>>(
                        end - start);
        state.SetIterationTime(elapsed_seconds.count());
        i++;
    }
    state.SetItemsProcessed(i);
}

static void BM_Get_Best_Bid(benchmark::State &state) {
    spdlog::set_level(spdlog::level::err);
    const int SECURITY_ID = 1;
    const std::string USERNAME = "test";
    Security apl("apple", "AAPL", SECURITY_ID);
    uint64_t i = 0;
    for (auto _: state) {
        auto book = createOrderBook();
        Order bid(OrderCore(USERNAME, 1), 500, 100, true);
        book.AddOrder(bid);
        auto start = std::chrono::high_resolution_clock::now();
        book.GetBestBidPrice();
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds =
                std::chrono::duration_cast<std::chrono::duration<double>>(
                        end - start);
        state.SetIterationTime(elapsed_seconds.count());
        i++;
    }
    state.SetItemsProcessed(i);
}

static void BM_Add_Order_Existing_Limit(benchmark::State &state) {
    spdlog::set_level(spdlog::level::err);
    const int SECURITY_ID = 1;
    const std::string USERNAME = "test";
    Security apl("apple", "AAPL", SECURITY_ID);
    uint64_t i = 0;
    for (auto _: state) {
        auto book = createOrderBook();
        book.AddOrder(Order(OrderCore(USERNAME, 1), 500, 100, true));
        Order bid(OrderCore(USERNAME, 1), 500, 100, true);
        auto start = std::chrono::high_resolution_clock::now();
        book.AddOrder(bid);
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds =
                std::chrono::duration_cast<std::chrono::duration<double>>(
                        end - start);
        state.SetIterationTime(elapsed_seconds.count());
        i++;
    }
    state.SetItemsProcessed(i);
}

static void BM_Add_Order_New_Limit(benchmark::State &state) {
    spdlog::set_level(spdlog::level::err);
    const int SECURITY_ID = 1;
    const std::string USERNAME = "test";
    Security apl("apple", "AAPL", SECURITY_ID);
    uint64_t i = 0;
    for (auto _: state) {
        auto book = createOrderBook();
        Order bid(Order(OrderCore(USERNAME, 1), 500, 100, true));
        auto start = std::chrono::high_resolution_clock::now();
        book.AddOrder(bid);
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds =
                std::chrono::duration_cast<std::chrono::duration<double>>(
                        end - start);
        state.SetIterationTime(elapsed_seconds.count());
        i++;
    }
    state.SetItemsProcessed(i);
}

static void BM_Remove_Order(benchmark::State &state) {
    spdlog::set_level(spdlog::level::err);
    const int SECURITY_ID = 1;
    const std::string USERNAME = "test";
    Security apl("apple", "AAPL", SECURITY_ID);
    uint64_t i = 0;
    for (auto _: state) {
        auto book = createOrderBook();
        Order order = Order(OrderCore(USERNAME, 1), 500, 100, true);
        book.AddOrder(order);
        auto start = std::chrono::high_resolution_clock::now();
        book.RemoveOrder(order.OrderId());
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds =
                std::chrono::duration_cast<std::chrono::duration<double>>(
                        end - start);
        state.SetIterationTime(elapsed_seconds.count());
        i++;
    }
    state.SetItemsProcessed(i);
}

static void BM_AddCrossing_Orders(benchmark::State &state) {
    spdlog::set_level(spdlog::level::err);
    const int SECURITY_ID = 1;
    const std::string USERNAME = "test";
    Security apl("apple", "AAPL", SECURITY_ID);
    uint64_t i = 0;
    for (auto _: state) {
        auto book = createOrderBook();
        Order bid = Order(OrderCore(USERNAME, 1), 501, 100, true);
        book.AddOrder(bid);
        Order ask = Order(OrderCore(USERNAME, 1), 500, 100, false);
        auto start = std::chrono::high_resolution_clock::now();
        book.AddOrder(ask);
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds =
                std::chrono::duration_cast<std::chrono::duration<double>>(
                        end - start);
        state.SetIterationTime(elapsed_seconds.count());
        i++;
    }
    state.SetItemsProcessed(i);
}

static void BM_Run_Simulation(benchmark::State &state) {
    spdlog::set_level(spdlog::level::err);
    const int SECURITY_ID = 1;
    const std::string USERNAME = "test";
    Security apl("apple", "AAPL", SECURITY_ID);
    auto book = createOrderBook();

    std::random_device device_random_;
    std::default_random_engine generator_(device_random_());
    std::bernoulli_distribution boolDist(0.5);
    std::normal_distribution<> quantityDist(100, 10);

    Order firstBid(OrderCore(USERNAME, SECURITY_ID), 497, 2500, true);
    Order firstAsk(OrderCore(USERNAME, SECURITY_ID), 503, 2500, false);
    book.AddOrder(firstBid);
    book.AddOrder(firstAsk);
    long lastAsk = 503;
    long lastBid = 497;

    uint64_t i = 0;

    for (auto _: state) {
        long bestBid = book.GetBestBidPrice().get_value_or(lastBid);
        long bestAsk = book.GetBestAskPrice().get_value_or(lastAsk);
        long MIN_DEVIANCE = 1 + rand() % (bestAsk - bestBid);
        double midPrice = (bestAsk + bestBid) / (double) 2;
        // 2 limit orders
        double bidMean = (bestBid + midPrice) / 2;
        double askMean = (bestAsk + midPrice) / 2;
        std::normal_distribution<> bidPriceDist(bidMean, 5);
        std::normal_distribution<> sellPriceDist(askMean, 5);
        double priceDouble = bidPriceDist(generator_);
        long price = round(priceDouble);
        double quantity = quantityDist(generator_);

        auto start = std::chrono::high_resolution_clock::now();
        for (int j = 0; j < 2; j++) {
            if (midPrice - priceDouble < MIN_DEVIANCE && priceDouble - midPrice < MIN_DEVIANCE) {
                bool isBuy = boolDist(generator_);
                isBuy
                ? book.PlaceMarketBuyOrder(quantityDist(generator_) / 2)
                : book.PlaceMarketSellOrder(
                        quantityDist(generator_) / 2);
                i++;
            } else if (price > bestAsk) {
                Order ask(OrderCore(USERNAME, SECURITY_ID), price, quantity, false);
                book.AddOrder(ask);
                lastAsk = price + 3;
                i++;
            } else if (price < bestBid) {
                Order bid(OrderCore(USERNAME, SECURITY_ID), price, quantity, true);
                book.AddOrder(bid);
                lastBid = price - 3;
                i++;
            } else {
                if (price < midPrice) {
                    Order bid(OrderCore(USERNAME, SECURITY_ID), price, quantity, true);
                    book.AddOrder(bid);
                    lastBid = price - 3;
                    i++;
                } else if (price > midPrice) {
                    Order ask(OrderCore(USERNAME, SECURITY_ID), price, quantity, false);
                    book.AddOrder(ask);
                    lastAsk = price + 3;
                    i++;
                }
            }
            priceDouble = sellPriceDist(generator_);
            price = round(priceDouble);
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds =
                std::chrono::duration_cast<std::chrono::duration<double>>(
                        end - start);
        state.SetIterationTime(elapsed_seconds.count());
    }
    state.SetItemsProcessed(i);
}


BENCHMARK(BM_PlaceMarketOrder)->UseManualTime();
BENCHMARK(BM_PlaceMarketOrderAcross3Bids)->UseManualTime();
BENCHMARK(BM_Get_Order)->UseManualTime();
BENCHMARK(BM_Get_Best_Bid)->UseManualTime();
BENCHMARK(BM_Run_Simulation)->UseManualTime();
BENCHMARK(BM_Add_Order_New_Limit)->UseManualTime();
BENCHMARK(BM_Add_Order_Existing_Limit)->UseManualTime();
BENCHMARK(BM_Remove_Order)->UseManualTime();
BENCHMARK(BM_AddCrossing_Orders)->UseManualTime();

BENCHMARK_MAIN();
