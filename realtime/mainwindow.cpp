#include "mainwindow.h"
#include <QElapsedTimer>
#include "./ui_mainwindow.h"

#include "Security.h"
#include "QOrderBook.h"

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow) {
    timer.start();
    ui->setupUi(this);
    ui->plot->addGraph();
    ui->plot->graph(0)->setPen(QPen(QColor(40, 110, 255)));
    ui->plot->addGraph(); // red line
    ui->plot->graph(1)->setPen(QPen(QColor(255, 110, 40)));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->plot->xAxis->setTicker(timeTicker);
    ui->plot->axisRect()->setupFullAxesBox();
    ui->plot->yAxis->setRange(490, 510);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->plot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->plot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->plot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->plot->yAxis2, SLOT(setRange(QCPRange)));



// setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
//    QTimer *dataTimer = new QTimer(this);
//    connect(dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
//    dataTimer->start(0); // Interval 0 means to refresh as fast as possible

//    static QTime time(QTime::currentTime());
//    static QElapsedTimer time();




//    ui->plot->addGraph();
//    ui->plot->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
//    ui->plot->graph(0)->setLineStyle(QCPGraph::lsNone); // no line in between points.
}

void MainWindow::realtimeDataSlot() {
    // calculate two new data points:
    double key = timer.elapsed() / 1000.0; // time elapsed since start of demo, in seconds
    static double lastPointKey = 0;
    if (key - lastPointKey > 0.002) // at most add point every 2 ms
    {
        // add data to lines:
        ui->plot->graph(0)->addData(key, qSin(key) + QRandomGenerator::global()->generate() / (double) RAND_MAX * 1 *
                                                     qSin(key / 0.3843));
        ui->plot->graph(1)->addData(key, qCos(key) + QRandomGenerator::global()->generate() / (double) RAND_MAX * 0.5 *
                                                     qSin(key / 0.4364));
        // rescale value (vertical) axis to fit the current data:
        ui->plot->graph(0)->rescaleValueAxis(true);
        ui->plot->graph(1)->rescaleValueAxis(true);
        lastPointKey = key;
    }
// make key axis range scroll with the data (at a constant range size of 8):
    ui->plot->xAxis->setRange(key, 8, Qt::AlignRight);
    ui->plot->replot();

// calculate frames per second:
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key - lastFpsKey > 2) // average fps over 2 seconds
    {

        statusBar()->showMessage(
                QString("%1 FPS, Total Data points: %2")
                        .arg(frameCount / (key - lastFpsKey), 0, 'f', 0)
                        .arg(ui->plot->graph(0)->data()->size() + ui->plot->graph(1)->data()->size()), 0);
        lastFpsKey = key;
        frameCount = 0;
    }
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::addPoint(double x, double y) {
    qv_x.append(x);
    qv_y.append(y);
}

void MainWindow::clearData() {
    qv_x.clear();
    qv_y.clear();
}

void MainWindow::plot() {
    ui->plot->graph(0)->setData(qv_x, qv_y);
    ui->plot->graph()->rescaleAxes(true);   // rescale the axis on each plot
    ui->plot->replot();
    ui->plot->update();
}


//void MainWindow::on_btn_add_clicked() {
//    addPoint(ui->bx_x->value(), ui->bx_y->value());
//    plot();
//}
//
//
//void MainWindow::on_btn_clear_clicked() {
//    clearData();
//    plot();
//}

void MainWindow::handleDataFetched(std::pair<long, long> data) {
    double key = timer.elapsed() / 1000.0; // time elapsed since start of demo, in seconds

    if (data.first < 100 || data.second < 100)
        return;
    ui->plot->graph(0)->addData(key, data.first);
    ui->plot->graph(1)->addData(key, data.second);
    double sum = 0, count = 0;
    auto plotData = ui->plot->graph(0)->data();
    for (int i = 0; i < plotData->size(); ++i) {
        sum += plotData->at(i)->value;
        count++;
    }
    double avg = sum / count;
    ui->plot->yAxis->setRange(avg - 10, avg + 10);
    ui->plot->xAxis->setRange(key, 8, Qt::AlignRight);
    ui->plot->replot();

    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key - lastFpsKey > 2) // average fps over 2 seconds
    {

        statusBar()->showMessage(
                QString("%1 FPS, Total Data points: %2")
                        .arg(frameCount / (key - lastFpsKey), 0, 'f', 0)
                        .arg(ui->plot->graph(0)->data()->size() + ui->plot->graph(1)->data()->size()), 0);
        lastFpsKey = key;
        frameCount = 0;
    }
}
