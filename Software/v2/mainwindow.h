#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "qcustomplot.h"
#include "commhandler.h"
#include "hidhandler.h"

#include <iostream>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnectionChanged(bool connected);
    void onConfigChanged(uint32_t logInterval, uint32_t maxCurrent, bool logOc);
    void onDataAvailable(uint32_t timestamp, float voltage, float current, float power);

    void on_pushButton_connect_clicked();

    void on_pushButton_clearDisplays_clicked();

private:
    void configureAppStyle(bool dark);
    void setPlotColors(QCustomPlot* plot, QColor background, QColor foreground);
    void configureVoltageChart();
    void configureCurrentChart();
    void configurePowerChart();

    constexpr static float MAX_VOLTAGE = 5.5;  //  5.5V
    constexpr static float MAX_CURRENT = 3000; // 3000mA
    constexpr static float MIN_CURRENT = 100;  //  100mA
    constexpr static float MAX_POWER = MAX_VOLTAGE * MAX_CURRENT;
    constexpr static float MIN_POWER = MAX_VOLTAGE * MIN_CURRENT;

    Ui::MainWindow *ui;

    QFont monoFont;
    QPalette lightPalette;
    QPalette darkPalette;
    bool darkMode;

    CommHandler* connection;

    double minU = MAX_VOLTAGE;
    double maxU = 0;
    double minI = MAX_CURRENT;
    double maxI = 0;
    double minP = MAX_POWER;
    double maxP = 0;

    bool logOcEvents = true;
    float iLimit = 500;
};
#endif // MAINWINDOW_H
