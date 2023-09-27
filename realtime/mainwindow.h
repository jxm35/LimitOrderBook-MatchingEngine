#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QElapsedTimer>

#include "QOrderBook.h"
#include "qcustomplot.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

private slots:

    void on_bx_sell_clicked();

    void handleDataFetched(GraphData data); // Slot to handle fetched data


private:
    Ui::MainWindow *ui;
    QElapsedTimer timer;
    QVector<double> qv_x, qv_y;
    QCPBars *bidLimits;
    QCPBars *askLimits;
    QCPBars *volumeBars;
};

#endif // MAINWINDOW_H
