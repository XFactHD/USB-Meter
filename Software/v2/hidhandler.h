#ifndef HIDHANDLER_H
#define HIDHANDLER_H

#include <QObject>
#include <QList>
#include <QDebug>
#include <QThread>
#include <QTimer>

#include "qhidapi/qhidapi.h"
#include "commhandler.h"

#include <iostream>

#define USB_VID 0xF055
#define USB_PID 0x0000

class HIDReader;
class HIDHandler : public CommHandler
{
    Q_OBJECT
public:
    explicit HIDHandler(QObject *parent = nullptr);

    virtual bool connectDevice() override;
    virtual void disconnectDevice() override;
    virtual bool isConnected() override;

    virtual bool sendByte(uint8_t data) override;
    virtual bool sendString(QString data) override;

    virtual void shutdown() override;

private slots:
    void on_data_available(QByteArray data);
    void on_device_errored(QString error);

private:
    bool connected = false;
    QHidApi* hid;
    uint32_t deviceId;
    HIDReader* readerThread;
};

class HIDReader : public QThread
{
    Q_OBJECT
public:
    explicit HIDReader(QHidApi* hid, uint32_t deviceId, QObject* parent = nullptr) : QThread(parent), hid(hid), deviceId(deviceId) {}

    void run() override;

signals:
    void on_data_received(QByteArray data);
    void on_device_errored(QString error);

private slots:
    void on_timer_timeout();

private:
    bool running = true;
    QHidApi* hid;
    uint32_t deviceId;

    QTimer* timer;
};

#endif // HIDHANDLER_H
