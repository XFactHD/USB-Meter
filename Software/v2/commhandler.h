#ifndef COMMHANDLER_H
#define COMMHANDLER_H

#include <QObject>
#include <QString>

#define CON_TIMEOUT 2000

#define CMD_START 0xA0
#define CMD_START_ACK 0xA5
#define CMD_CFG 0xB0
#define CMD_CFG_ACK 0xB5
#define CMD_DATA 0xC0
#define CMD_DATA_ACK 0xC5
#define CMD_RC_ILIM 0xD0
#define CMD_RC_OUTPUT 0xD1
#define CMD_RC_ACK 0xE0
#define CMD_RC_NACK 0xE5
#define CMD_STOP 0xF0

class CommHandler : public QObject
{
    Q_OBJECT
public:
    explicit CommHandler(QObject* parent = nullptr) : QObject(parent) {}

    virtual bool connectDevice() = 0;
    virtual void disconnectDevice() = 0;
    virtual bool isConnected() = 0;

    virtual bool sendByte(uint8_t data) = 0;
    virtual bool sendString(QString data) = 0;

    virtual void shutdown() = 0;

signals:
    void onConnectionChanged(bool connected);
    void onConfigChanged(uint32_t logInterval, uint32_t maxCurrent, bool logOc);
    void onDataReceived(long timestamp, float voltage, float current, float power);
};

#endif // COMMHANDLER_H
