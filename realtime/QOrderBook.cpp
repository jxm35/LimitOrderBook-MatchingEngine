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
}

void QOrderBook::startOrderBook() {
    pthread_t thread;
    pthread_create(&thread, nullptr, threadHelper, this);
}

void *QOrderBook::threadHelper(void *context) {
    ((QOrderBook *) context)->runSimulation();
    return nullptr;
}

void *QOrderBook::runSimulation() {
    const int SECURITY_ID = 1;
    const std::string USERNAME = "test";
    const long MIN_DEVIANCE = 3;

    std::random_device device_random_;
    std::default_random_engine generator_(device_random_());
    std::bernoulli_distribution boolDist(0.5);
    std::normal_distribution<> quantityDist(1000, 10);
    boolDist(generator_);

    Order firstBid(OrderCore(USERNAME, SECURITY_ID), 499, quantityDist(generator_), true);
    Order firstAsk(OrderCore(USERNAME, SECURITY_ID), 500, quantityDist(generator_), false);
    book_.AddOrder(firstBid);
    book_.AddOrder(firstAsk);

    lastAsk = 500;
    lastBid = 499;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 2000000; i++) {
        usleep(10000); // sleep 10ms
        long bestBid = book_.GetBestBidPrice().get_value_or(lastBid);
        long bestAsk = book_.GetBestAskPrice().get_value_or(lastAsk);
        if (bestAsk < 400 || bestBid < 400 || lastBid < 400 || lastAsk < 400) {
            std::cout << "what";
        }
        double midPrice = (bestAsk + bestBid) / (double) 2;
        // 2 limit orders
        std::normal_distribution<> priceDist(midPrice, 8);
        for (int j = 0; j < 2; j++) {
            double priceDouble = priceDist(generator_);
            long price = floor(priceDouble);
            if (midPrice - price < MIN_DEVIANCE && price - midPrice < MIN_DEVIANCE)
                continue;
            if (price > bestAsk) {
                Order ask(OrderCore(USERNAME, SECURITY_ID), price, quantityDist(generator_), false);
                book_.AddOrder(ask);
                lastAsk = price - 3;
            } else if (price < bestBid) {
                Order bid(OrderCore(USERNAME, SECURITY_ID), price, quantityDist(generator_), true);
                book_.AddOrder(bid);
                lastBid = price - 3;
            } else {
                // make a market order
            }
        }
        // market order
        bool isBuy = boolDist(generator_);
        Order order(OrderCore(USERNAME, SECURITY_ID), isBuy ? bestAsk : bestBid, quantityDist(generator_), isBuy);
        book_.AddOrder(order);

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

void QOrderBook::fetchData() {
    long bestBid = book_.GetBestBidPrice().get_value_or(lastBid);
    long bestAsk = book_.GetBestAskPrice().get_value_or(lastAsk);
    if (bestAsk < 400 || bestBid < 400 || lastBid < 400 || lastAsk < 400) {
        std::cout << "what";
    }
    emit dataFetched({bestBid, bestAsk});
}


void QOrderBook::startTimer() {
    std::cout << "timer started " << std::endl;
    dataPollTimer->start(50); // Poll every 100 milliseconds
}

