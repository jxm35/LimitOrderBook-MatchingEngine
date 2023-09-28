#pragma once


#include <QObject>
#include <QTimer>
#include <map>
#include "OrderBook.h"

struct GraphData {
    long bidPrice;
    long askPrice;
    std::map<long, uint32_t> bidQuantities;
    std::map<long, uint32_t> askQuantities;
    long orders;
    long spread;
    long bestBid;
    long bestAsk;
    long bestBidDepth;
    long bestAskDepth;
    long volume;
};

class QOrderBook : public QObject {
Q_OBJECT

public:
    explicit QOrderBook(Security sec);


public slots:

    void startTimer();

    void fetchData(); // Slot to fetch data from the order book

    void startOrderBook();

    void handleSellButton();

    void handleBuyButton();


    void stopOrderBook();

    void processData(); // Slot to receive data from the Qt application

signals:

    void dataFetched(GraphData data); // Signal to notify the main thread that data is ready


private:
    static void *simulationHelper(void *context);

    void *runSimulation();

    static void *sellHelper(void *context);

    void *applySellPressure();

    static void *buyHelper(void *context);

    void *applyBuyPressure();

    std::mutex mutex;
    OrderBook book_;
    QTimer *dataPollTimer; // QTimer for periodic data polling
    bool running_;
    long lastAsk, lastBid;
    long ordersMatched;
};