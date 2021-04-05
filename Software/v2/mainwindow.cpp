#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    monoFont = QFont("Monospaced");
    monoFont.setStyleHint(QFont::Monospace);
    setFont(monoFont);

    ui->setupUi(this);

    //Disable window resizing and maximizing
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    //Set window icon
    setWindowIcon(QIcon("icon.ico"));

    //Configure application details
    QCoreApplication::setOrganizationName("dc");
    QCoreApplication::setApplicationName("USB-PowerLogger");

    //Configure dark mode palette
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::black);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(63, 63, 63));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    darkMode = true;//QSettings().value("app_dark_mode").toBool();
    configureAppStyle(darkMode);

    configureVoltageChart();
    configureCurrentChart();
    configurePowerChart();

    connection = new HIDHandler(this);
    connect(connection, &CommHandler::onConnectionChanged, this, &MainWindow::onConnectionChanged);
    connect(connection, &CommHandler::onConfigChanged, this, &MainWindow::onConfigChanged);
    connect(connection, &CommHandler::onDataReceived, this, &MainWindow::onDataAvailable);
}

void MainWindow::configureAppStyle(bool dark)
{
    if (dark) {
        setPalette(darkPalette);

        ui->menuMain->setPalette(darkPalette);

        QColor darkBg(63, 63, 63);
        setPlotColors(ui->plotVoltage, darkBg, Qt::white);
        setPlotColors(ui->plotCurrent, darkBg, Qt::white);
        setPlotColors(ui->plotPower, darkBg, Qt::white);
    }
    else {
        setPalette(lightPalette);

        ui->menuMain->setPalette(lightPalette);

        setPlotColors(ui->plotVoltage, Qt::white, Qt::black);
        setPlotColors(ui->plotCurrent, Qt::white, Qt::black);
        setPlotColors(ui->plotPower, Qt::white, Qt::black);
    }
}

void MainWindow::setPlotColors(QCustomPlot* plot, QColor background, QColor foreground)
{
    plot->setBackground(QBrush(background));
    plot->xAxis->setBasePen(QPen(foreground));
    plot->xAxis->setLabelColor(foreground);
    plot->xAxis->setTickPen(QPen(foreground));
    plot->xAxis->setSubTickPen(QPen(foreground));
    plot->xAxis->setTickLabelColor(foreground);
    plot->yAxis->setBasePen(QPen(foreground));
    plot->yAxis->setLabelColor(foreground);
    plot->yAxis->setTickPen(QPen(foreground));
    plot->yAxis->setSubTickPen(QPen(foreground));
    plot->yAxis->setTickLabelColor(foreground);
}

void MainWindow::configureVoltageChart()
{
    ui->plotVoltage->xAxis->setRange(0, 5, Qt::AlignLeft);
    ui->plotVoltage->yAxis->setRange(0, MAX_VOLTAGE);

    ui->plotVoltage->axisRect()->setAutoMargins(QCP::msTop | QCP::msRight);
    ui->plotVoltage->axisRect()->setMargins(QMargins(80, 0, 0, 50));

    QSharedPointer<QCPAxisTickerFixed> voltageTicker(new QCPAxisTickerFixed);
    voltageTicker->setTickStep(.5);
    ui->plotVoltage->yAxis->setTicker(voltageTicker);

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%s.%z");
    timeTicker->setFieldWidth(QCPAxisTickerTime::tuSeconds, 1);
    timeTicker->setFieldWidth(QCPAxisTickerTime::tuMilliseconds, 1);
    ui->plotVoltage->xAxis->setTicker(timeTicker);

    ui->plotVoltage->xAxis->setTickLabelFont(monoFont);
    ui->plotVoltage->yAxis->setTickLabelFont(monoFont);

    ui->plotVoltage->xAxis->setLabel("Time (s)");
    ui->plotVoltage->yAxis->setLabel("Voltage (V)");

    ui->plotVoltage->addGraph();
    ui->plotVoltage->graph(0)->setName("Voltage (V)");
    ui->plotVoltage->graph(0)->setPen(QPen(Qt::red));
}

void MainWindow::configureCurrentChart()
{
    ui->plotCurrent->xAxis->setRange(0, 5, Qt::AlignLeft);
    ui->plotCurrent->yAxis->setRange(0, MAX_CURRENT);

    ui->plotCurrent->axisRect()->setAutoMargins(QCP::msTop | QCP::msRight);
    ui->plotCurrent->axisRect()->setMargins(QMargins(80, 0, 0, 50));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%s.%z");
    timeTicker->setFieldWidth(QCPAxisTickerTime::tuSeconds, 1);
    timeTicker->setFieldWidth(QCPAxisTickerTime::tuMilliseconds, 1);
    ui->plotCurrent->xAxis->setTicker(timeTicker);

    ui->plotCurrent->xAxis->setTickLabelFont(monoFont);
    ui->plotCurrent->yAxis->setTickLabelFont(monoFont);

    ui->plotCurrent->xAxis->setLabel("Time (s)");
    ui->plotCurrent->yAxis->setLabel("Current (mA)");

    ui->plotCurrent->addGraph();
    ui->plotCurrent->graph(0)->setName("Current (mA)");
    ui->plotCurrent->graph(0)->setPen(QPen(Qt::red));
}

void MainWindow::configurePowerChart()
{
    ui->plotPower->xAxis->setRange(0, 5, Qt::AlignLeft);
    ui->plotPower->yAxis->setRange(0, MAX_POWER);
    ui->plotPower->yAxis->ticker()->setTickCount(10);

    ui->plotPower->axisRect()->setAutoMargins(QCP::msTop | QCP::msRight);
    ui->plotPower->axisRect()->setMargins(QMargins(80, 0, 0, 50));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%s.%z");
    timeTicker->setFieldWidth(QCPAxisTickerTime::tuSeconds, 1);
    timeTicker->setFieldWidth(QCPAxisTickerTime::tuMilliseconds, 1);
    ui->plotPower->xAxis->setTicker(timeTicker);

    ui->plotPower->xAxis->setTickLabelFont(monoFont);
    ui->plotPower->yAxis->setTickLabelFont(monoFont);

    ui->plotPower->xAxis->setLabel("Time (s)");
    ui->plotPower->yAxis->setLabel("Power (mW)");

    ui->plotPower->addGraph();
    ui->plotPower->graph(0)->setName("Power (mW)");
    ui->plotPower->graph(0)->setPen(QPen(Qt::red));
}

MainWindow::~MainWindow()
{
    connection->disconnectDevice();
    delete connection;

    delete ui;
}



void MainWindow::onConnectionChanged(bool connected)
{
    ui->pushButton_clearDisplays->setDisabled(connected);
    ui->pushButton_connect->setText(connected ? "Disconnect" : "Connect");
}

void MainWindow::onConfigChanged(uint32_t logInterval, uint32_t maxCurrent, bool logOc)
{
    (void)logInterval;
    iLimit = maxCurrent;
    logOcEvents = logOc;

    float max = std::max(std::min(std::min(float(maxCurrent) * 1.5F, float(maxCurrent) + 500.0F), MAX_CURRENT), MIN_CURRENT);
    ui->plotCurrent->yAxis->setRange(0, max);
    ui->plotPower->yAxis->setRange(0, max * MAX_VOLTAGE);
}

void MainWindow::onDataAvailable(uint32_t timestamp, float voltage, float current, float power)
{
    //Voltage displays
    QString text = QString("%1V").arg(voltage, 0, 'f', 2);
    ui->lineEdit_uCurr->setText(text);
    if (voltage < minU) {
        minU = voltage;
        ui->lineEdit_uMin->setText(text);
    }
    if (current > maxU) {
        maxU = current;
        ui->lineEdit_uMax->setText(text);
    }

    //Current displays
    text = QString("%1mA").arg(current, 0, 'f', 0);
    ui->lineEdit_iCurr->setText(text);
    if (current < minI) {
        minI = current;
        ui->lineEdit_iMin->setText(text);
    }
    if (current > maxI) {
        maxI = current;
        ui->lineEdit_iMax->setText(text);
    }

    //Power displays
    text = QString("%1mW").arg(power, 0, 'f', 0);
    ui->lineEdit_pCurr->setText(text);
    if (power < minP) {
        minP = power;
        ui->lineEdit_pMin->setText(text);
    }
    if (power > maxP) {
        maxP = power;
        ui->lineEdit_pMax->setText(text);
    }

    //Graphs
    double time = timestamp / 1000.0;

    ui->plotVoltage->graph(0)->addData(time, voltage);
    ui->plotCurrent->graph(0)->addData(time, current);
    ui->plotPower->graph(0)->addData(time, power);

    if (time > 5.0) {
        ui->plotVoltage->xAxis->setRange(time, 5, Qt::AlignRight);
        ui->plotCurrent->xAxis->setRange(time, 5, Qt::AlignRight);
        ui->plotPower->xAxis->setRange(time, 5, Qt::AlignRight);
    }

    if (logOcEvents && current > iLimit) {
        QCPItemRect* marker = new QCPItemRect(ui->plotCurrent);
        marker->setSelectable(false);
        marker->setBrush(QBrush(Qt::red));
        marker->topLeft->setType(QCPItemPosition::ptPlotCoords);
        marker->topLeft->setCoords(time - 0.05, current + 50);
        marker->bottomRight->setType(QCPItemPosition::ptPlotCoords);
        marker->bottomRight->setCoords(time + 0.05, current - 50);
    }

    ui->plotVoltage->replot();
    ui->plotCurrent->replot();
    ui->plotPower->replot();
}

void MainWindow::on_pushButton_connect_clicked()
{
    if (connection->isConnected()) {
        connection->disconnectDevice();
    }
    else {
        connection->connectDevice();
    }
}

void MainWindow::on_pushButton_clearDisplays_clicked()
{
    minU = MAX_VOLTAGE;
    maxU = 0;
    minI = MAX_CURRENT;
    maxI = 0;
    minP = MAX_POWER;
    maxP = 0;

    ui->lineEdit_uCurr->setText("0.0V");
    ui->lineEdit_uMin->setText("0.0V");
    ui->lineEdit_uMax->setText("0.0V");

    ui->lineEdit_iCurr->setText("0mA");
    ui->lineEdit_iMin->setText("0mA");
    ui->lineEdit_iMax->setText("0mA");

    ui->lineEdit_pCurr->setText("0mW");
    ui->lineEdit_pMin->setText("0mW");
    ui->lineEdit_pMax->setText("0mW");

    ui->plotVoltage->graph(0)->data()->clear();
    ui->plotVoltage->xAxis->setRange(0, 5);
    ui->plotVoltage->replot();

    ui->plotCurrent->graph(0)->data()->clear();
    ui->plotCurrent->xAxis->setRange(0, 5);
    ui->plotCurrent->yAxis->setRange(0, MAX_CURRENT);
    ui->plotCurrent->clearItems();
    ui->plotCurrent->replot();

    ui->plotPower->graph(0)->data()->clear();
    ui->plotPower->xAxis->setRange(0, 5);
    ui->plotPower->yAxis->setRange(0, MAX_POWER);
    ui->plotPower->replot();
}
