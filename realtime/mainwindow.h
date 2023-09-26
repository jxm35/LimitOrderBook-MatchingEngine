#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QElapsedTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

    void addPoint(double x, double y);

    void clearData();

    void plot();

private slots:

//    void on_btn_add_clicked();
//
//    void on_btn_clear_clicked();

    void realtimeDataSlot();

    void handleDataFetched(std::pair<long, long> data); // Slot to handle fetched data


private:
    Ui::MainWindow *ui;
    QElapsedTimer timer;
    QVector<double> qv_x, qv_y;
};

#endif // MAINWINDOW_H
