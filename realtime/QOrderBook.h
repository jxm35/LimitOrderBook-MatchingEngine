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

    void *runSimulation();

    static void *threadHelper(void *context);


public slots:

    void startTimer();

    void fetchData(); // Slot to fetch data from the order book

    void startOrderBook();


    void stopOrderBook();

    void processData(); // Slot to receive data from the Qt application

signals:

    void dataFetched(GraphData data); // Signal to notify the main thread that data is ready


private:
    OrderBook book_;
    QTimer *dataPollTimer; // QTimer for periodic data polling
    bool running_;
    long lastAsk, lastBid;
    long ordersMatched;
};