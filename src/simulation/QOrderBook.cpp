#include "QOrderbook.h"
#include "Security.h"
#include <iostream>
#include <chrono>
#include <pthread.h>
#include <random>

QOrderBook::QOrderBook(Security sec) : book_(sec) {
    dataPollTimer = new QTimer(this);
    connect(dataPollTimer, SIGNAL(timeout()), this, SLOT(fetchData()));
    running_ = false;
    ordersMatched = 0;
}

void QOrderBook::handleSellButton() {
    pthread_t thread;
    pthread_create(&thread, nullptr, sellHelper, this);
}

void *QOrderBook::sellHelper(void *context) {
    ((QOrderBook *) context)->applySellPressure();
    return nullptr;
}

void QOrderBook::handleBuyButton() {
    pthread_t thread;
    pthread_create(&thread, nullptr, buyHelper, this);
}

void *QOrderBook::buyHelper(void *context) {
    ((QOrderBook *) context)->applyBuyPressure();
    return nullptr;
}

void QOrderBook::startOrderBook() {
    pthread_t thread;
    pthread_create(&thread, nullptr, simulationHelper, this);
}

void *QOrderBook::simulationHelper(void *context) {
    ((QOrderBook *) context)->runSimulation();
    return nullptr;
}

void *QOrderBook::runSimulation() {
    const int SECURITY_ID = 1;
    const std::string USERNAME = "test";

    std::random_device device_random_;
    std::default_random_engine generator_(device_random_());
    std::bernoulli_distribution boolDist(0.5);
    std::normal_distribution<> quantityDist(100, 10);

    Order firstBid(OrderCore(USERNAME, SECURITY_ID), 497, 2500, true);
    Order firstAsk(OrderCore(USERNAME, SECURITY_ID), 503, 2500, false);
    book_.AddOrder(firstBid);
    book_.AddOrder(firstAsk);

    lastAsk = 503;
    lastBid = 497;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 2000000; i++) {
        usleep(10000); // sleep 10ms
        std::lock_guard<std::mutex> lock(mutex);
        long bestBid = book_.GetBestBidPrice().get_value_or(lastBid);
        long bestAsk = book_.GetBestAskPrice().get_value_or(lastAsk);
        long MIN_DEVIANCE = 1 + rand() % (bestAsk - bestBid);
        double midPrice = (bestAsk + bestBid) / (double) 2;
        // 2 limit orders
        double bidMean = (bestBid + midPrice) / 2;
        double askMean = (bestAsk + midPrice) / 2;
        std::normal_distribution<> bidPriceDist(bidMean, 5);
        std::normal_distribution<> sellPriceDist(askMean, 5);
        double priceDouble = bidPriceDist(generator_);
        long price = round(priceDouble);
        for (int j = 0; j < 2; j++) {

            if (midPrice - priceDouble < MIN_DEVIANCE && priceDouble - midPrice < MIN_DEVIANCE) {
                bool isBuy = boolDist(generator_);
                isBuy ? book_.PlaceMarketBuyOrder(quantityDist(generator_) / 2) : book_.PlaceMarketSellOrder(
                        quantityDist(generator_) / 2);
            } else if (price > bestAsk) {
                Order ask(OrderCore(USERNAME, SECURITY_ID), price, quantityDist(generator_), false);
                book_.AddOrder(ask);
                lastAsk = price + 3;
            } else if (price < bestBid) {
                Order bid(OrderCore(USERNAME, SECURITY_ID), price, quantityDist(generator_), true);
                book_.AddOrder(bid);
                lastBid = price - 3;
            } else {
                if (price < midPrice) {
                    Order bid(OrderCore(USERNAME, SECURITY_ID), price, quantityDist(generator_), true);
                    book_.AddOrder(bid);
                    lastBid = price - 3;
                } else if (price > midPrice) {
                    Order ask(OrderCore(USERNAME, SECURITY_ID), price, quantityDist(generator_), false);
                    book_.AddOrder(ask);
                    lastAsk = price + 3;
                }
            }
            priceDouble = sellPriceDist(generator_);
            price = round(priceDouble);
        }
        // check for orders to cancel
        if (i > 200) {
            double orderCount = book_.Count();
            const double A = 0.02, B = 0;

            std::list<OrderBookEntry> bids = book_.GetBidOrders();
            double bidImbalance = (double) bids.size() / orderCount;
            for (auto order: bids) {
                long dist = bestAsk - order.CurrentOrder().Price();
                double cancelProb = A * (5 * (1 - exp(-dist))) * (bidImbalance + B) * (1 / orderCount);
                std::bernoulli_distribution d(cancelProb);
                if (d(generator_)) {
//                    book_.RemoveOrder(order.CurrentOrder());
                }
            }

            std::list<OrderBookEntry> asks = book_.GetAskOrders();
            double askImbalance = (double) asks.size() / orderCount;
            for (auto order: asks) {
                long dist = order.CurrentOrder().Price() - bestBid;
                double cancelProb = A * (5 * (1 - exp(-dist))) * (askImbalance + B) * (1 / orderCount);
                std::bernoulli_distribution d(cancelProb);
                if (d(generator_)) {
//                    book_.RemoveOrder(order.CurrentOrder());
                }
            }
        }


        // market order
//        bool isBuy = boolDist(generator_);
//        Order order(OrderCore(USERNAME, SECURITY_ID), isBuy ? bestAsk : bestBid, quantityDist(generator_), isBuy);
//        book_.AddOrder(order);

        book_.Match();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Elapsed Time: " << duration.count() << "ms";
}

void QOrderBook::stopOrderBook() {

}

void QOrderBook::processData() {

}

void *QOrderBook::applySellPressure() {
    const int SECURITY_ID = 1;
    const std::string USERNAME = "test";
    std::random_device device_random_;
    std::default_random_engine generator_(device_random_());
    std::normal_distribution<> quantityDist(100, 10);
    std::bernoulli_distribution placeLimits(0.3);
    for (int i = 0; i < 500; i++) {
        usleep(2000); // sleep 1ms
        std::lock_guard<std::mutex> lock(mutex);
        book_.PlaceMarketSellOrder(quantityDist(generator_));
        long bestBid = book_.GetBestBidPrice().get_value_or(lastBid);
        long bestAsk = book_.GetBestAskPrice().get_value_or(lastAsk);
        if (placeLimits(generator_) && (bestAsk - bestBid) > 6) {
            double midPrice = (bestAsk + bestBid) / (double) 2;
            std::normal_distribution<> bidPriceDist((double) bestBid, 3);
            std::normal_distribution<> sellPriceDist(midPrice, 3);
            double sellPrice = sellPriceDist(generator_);
            double buyPrice = bidPriceDist(generator_);
            if (sellPrice < bestBid) {
                sellPrice = midPrice;
            }
            if (buyPrice > sellPrice) {
                buyPrice = bestBid;
            }
            Order ask(OrderCore(USERNAME, SECURITY_ID), sellPrice, quantityDist(generator_), false);
            book_.AddOrder(ask);
            lastAsk = sellPrice;
            Order bid(OrderCore(USERNAME, SECURITY_ID), buyPrice, quantityDist(generator_), true);
            book_.AddOrder(bid);
            lastBid = buyPrice;
            book_.Match();
        }
    }
}

void *QOrderBook::applyBuyPressure() {
    std::random_device device_random_;
    std::default_random_engine generator_(device_random_());
    std::normal_distribution<> quantityDist(100, 10);
    for (int i = 0; i < 500; i++) {
        usleep(2000); // sleep 1ms
        std::lock_guard<std::mutex> lock(mutex);
        book_.PlaceMarketBuyOrder(quantityDist(generator_));
    }
}

void QOrderBook::fetchData() {
    std::lock_guard<std::mutex> lock(mutex);
    long bestBid = book_.GetBestBidPrice().get_value_or(lastBid);
    long bestAsk = book_.GetBestAskPrice().get_value_or(lastAsk);
    auto bidLimit = book_.GetBestBidLimit();
    long bestBidDepth = 0;
    if (bidLimit.has_value()) {
        bestBidDepth = bidLimit.value()->GetOrderQuantity();
    }
    auto askLimit = book_.GetBestBidLimit();
    long bestAskDepth = 0;
    if (askLimit.has_value()) {
        bestAskDepth = askLimit.value()->GetOrderQuantity();
    }
    long volume = book_.GetOrdersMatched() - ordersMatched;
    ordersMatched += volume;
    GraphData data{
            bestBid,
            bestAsk,
            book_.GetBidQuantities(),
            book_.GetAskQuantities(),
            book_.Count(),
            book_.GetSpread().Spread().get_value_or(0),
            book_.GetBestBidPrice().get_value_or(lastBid),
            book_.GetBestAskPrice().get_value_or(lastAsk),
            bestBidDepth,
            bestAskDepth,
            volume,
    };
    emit dataFetched(data);
}


void QOrderBook::startTimer() {
    std::cout << "timer started " << std::endl;
    dataPollTimer->start(16); // Poll every 100 milliseconds
}

