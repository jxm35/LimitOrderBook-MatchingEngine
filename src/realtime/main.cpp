#include "mainwindow.h"
#include "Security.h"
#include "QOrderBook.h"

#include <QApplication>
#include <QThread>

int main(int argc, char *argv[]) {


    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    Security sec("apple", "aapl", 1);
    QOrderBook book(sec);

    QThread orderBookThread;
    book.moveToThread(&orderBookThread);
    orderBookThread.start();
    QMetaObject::invokeMethod(&book, "startOrderBook");
    QMetaObject::invokeMethod(&book, "startTimer");

    QObject::connect(&book, SIGNAL(dataFetched(GraphData)), &w,
                     SLOT(handleDataFetched(GraphData)));
    QObject::connect(&w, SIGNAL(applySellPressure()), &book,
                     SLOT(handleSellButton()));
    QObject::connect(&w, SIGNAL(applyBuyPressure()), &book,
                     SLOT(handleBuyButton()));

    return a.exec();
}
