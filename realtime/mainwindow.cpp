#include "mainwindow.h"
#include <QElapsedTimer>
#include <iostream>
#include "./ui_mainwindow.h"

#include "Security.h"
#include "QOrderBook.h"

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow) {
    timer.start();
    QColor green(10, 140, 70, 160), red(255, 40, 40), blue(40, 40, 255);
    ui->setupUi(this);

    // price time graph
    ui->price_graph->addGraph();
    ui->price_graph->graph(0)->setPen(QPen(green));
    ui->price_graph->addGraph(); // red line
    ui->price_graph->graph(1)->setPen(QPen(red));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->price_graph->xAxis->setTicker(timeTicker);
    ui->price_graph->axisRect()->setupFullAxesBox();
    ui->price_graph->yAxis->setRange(490, 510);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->price_graph->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->price_graph->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->price_graph->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->price_graph->yAxis2, SLOT(setRange(QCPRange)));

    //volume bar chart
    ui->volume->xAxis->setTicker(timeTicker);
    ui->volume->axisRect()->setupFullAxesBox();
    volumeBars = new QCPBars(ui->volume->xAxis, ui->volume->yAxis);
    volumeBars->setWidth(0.05);
    volumeBars->setPen(Qt::NoPen);
    volumeBars->setBrush(blue);


    // level depth bar chart
    bidLimits = new QCPBars(ui->heatmap->yAxis, ui->heatmap->xAxis);
    bidLimits->setWidth(0.5);
    bidLimits->setPen(Qt::NoPen);
    bidLimits->setBrush(green);

    askLimits = new QCPBars(ui->heatmap->yAxis, ui->heatmap->xAxis);
    askLimits->setWidth(0.5);
    askLimits->setPen(Qt::NoPen);
    askLimits->setBrush(red);
    ui->heatmap->yAxis->setRange(490, 510);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_bx_sell_clicked() {
    std::cout << "wazzup" << std::endl;
}

void MainWindow::handleDataFetched(GraphData data) {
    double key = timer.elapsed() / 1000.0; // time elapsed since start of demo, in seconds

    // price time chart
    if (data.bidPrice < 100 || data.askPrice < 100)
        return;
    ui->price_graph->graph(0)->addData(key, data.bidPrice);
    ui->price_graph->graph(1)->addData(key, data.askPrice);
    double sum = 0, count = 0;
    auto plotData = ui->price_graph->graph(0)->data();
    for (int i = 0; i < plotData->size(); ++i) {
        sum += plotData->at(i)->value;
        count++;
    }
    double avg = sum / count;
    ui->price_graph->yAxis->setRange(avg - 10, avg + 10);
    ui->heatmap->yAxis->setRange(avg - 10, avg + 10);
    ui->price_graph->xAxis->setRange(key, 8, Qt::AlignRight);
    ui->price_graph->replot();

    // volume bar chart
    volumeBars->addData(key, data.volume);
    ui->volume->xAxis->setRange(key, 8, Qt::AlignRight);
    ui->volume->yAxis->rescale();
    ui->volume->replot();



    // depth bar charts
    double start = ui->price_graph->yAxis->range().lower;
    double end = ui->price_graph->yAxis->range().upper;
    QVector<double> bidKeys(end - start + 1), bidValues(end - start + 1), askKeys(end - start + 1), askValues(
            end - start + 1);
    for (double i = 0; i < end - start; i++) {
        bidKeys[i] = start + i;
        askKeys[i] = start + i;
        if (data.bidQuantities.find(start + i) != data.bidQuantities.end()) {
            bidValues[i] = data.bidQuantities.at(start + i);
        } else {
            bidValues[i] = 0;
        }
        if (data.askQuantities.find(start + i) != data.askQuantities.end()) {
            askValues[i] = data.askQuantities.at(start + i);
        } else {
            askValues[i] = 0;
        }
    }
    bidLimits->setData(bidKeys, bidValues);
    askLimits->setData(askKeys, askValues);
    ui->heatmap->xAxis->rescale(true);
//    ui->heatmap->rescaleAxes();
    ui->heatmap->replot();

    ui->lb_total_orders->setText(QString("Order Count: %1").arg(data.orders));
    ui->lb_spread->setText(QString("Spread: %1").arg(data.spread));
    ui->lb_best_bid->setText(QString("Best Bid: %1 p").arg(data.bestBid));
    ui->lb_best_ask->setText(QString("Best Ask: %1p").arg(data.bestAsk));
    ui->lb_bid_depth->setText(QString("Best Bid Depth: %1").arg(data.bestBidDepth));
    ui->lb_ask_depth->setText(QString("Best Ask Depth: %1").arg(data.bestAskDepth));

    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key - lastFpsKey > 2) // average fps over 2 seconds
    {

        statusBar()->showMessage(
                QString("%1 FPS")
                        .arg(frameCount / (key - lastFpsKey), 0, 'f', 0), 0);
        lastFpsKey = key;
        frameCount = 0;
    }
}
