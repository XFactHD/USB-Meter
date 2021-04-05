#include "hidhandler.h"

HIDHandler::HIDHandler(QObject *parent) : CommHandler(parent) { }

void HIDHandler::shutdown() { }



bool HIDHandler::connectDevice()
{
    hid = new QHidApi(this);
    if (!(deviceId = hid->open(USB_VID, USB_PID))) {
        qCritical() << "Error while initializing HID lib!";
        delete hid;
        return false;
    }

    if (!sendByte(CMD_START)) {
        std::cout << "Failed to write CMD_START" << std::endl;

        hid->close(deviceId);
        delete hid;
        return false;
    }

    QByteArray data = hid->read(deviceId, 500);
    if (data.isEmpty() || uint8_t(data.at(0)) != CMD_START_ACK) {
        std::cout << "Failed to read CMD_START_ACK (Size: " << data.size() << ")" << std::endl;

        hid->close(deviceId);
        delete hid;
        return false;
    }
    data = hid->read(deviceId, 500);
    if (data.isEmpty() || uint8_t(data.at(0)) != CMD_CFG) {
        std::cout << "Failed to read CMD_CFG (Size: " << data.size() << ")" << std::endl;

        hid->close(deviceId);
        delete hid;
        return false;
    }
    on_data_available(data);

    if (!sendByte(CMD_CFG_ACK)) {
        std::cout << "Failed to write CMD_CFG_ACK" << std::endl;

        hid->close(deviceId);
        delete hid;
        return false;
    }

    std::cout << "HID connected" << std::endl;

    readerThread = new HIDReader(hid, deviceId, this);
    connect(readerThread, &HIDReader::on_data_received, this, &HIDHandler::on_data_available);
    connect(readerThread, &HIDReader::on_device_errored, this, &HIDHandler::on_device_errored);
    readerThread->start();

    connected = true;
    emit onConnectionChanged(true);

    return true;
}

void HIDHandler::disconnectDevice()
{
    if (connected) {
        connected = false;

        sendByte(0xF0);

        readerThread->requestInterruption();
        while (readerThread->isRunning());
        disconnect(readerThread, &HIDReader::on_data_received, this, &HIDHandler::on_data_available);
        delete readerThread;

        hid->close(deviceId);
        delete hid;
        deviceId = 0;

        emit onConnectionChanged(false);
    }
}

bool HIDHandler::isConnected()
{
    return connected;
}

bool HIDHandler::sendByte(uint8_t data)
{
    QByteArray out(1, 0); //Init with 0 as reportId
    out.insert(1, data);
    return hid->write(deviceId, out) != -1;
}

bool HIDHandler::sendString(QString data)
{
    QByteArray out(1, 0); //Init with 0 as reportId
    out.insert(1, data.toUtf8());
    return hid->write(deviceId, out) != -1;
}

void HIDHandler::on_data_available(QByteArray data)
{
    uint8_t cmd = uint8_t(data.at(0));
    if (cmd == CMD_DATA) {
        sendByte(CMD_DATA_ACK);

        uint8_t* bytes = reinterpret_cast<uint8_t*>(data.data());

        //FIXME: this is a stupid way to get the data out
        uint32_t timestamp = ((uint32_t*)(bytes + 1))[0];
        float voltage = ((float*)(bytes + 5))[0];
        float current = ((float*)(bytes + 5))[1];
        float power = ((float*)(bytes + 5))[2];

        emit onDataReceived(timestamp, voltage, current, power);
    }
    else if (cmd == CMD_CFG) {
        sendByte(CMD_CFG_ACK);

        uint8_t* bytes = reinterpret_cast<uint8_t*>(data.data());

        uint32_t logInterval = ((uint32_t*)(bytes + 1))[0];
        uint32_t maxCurrent = ((uint32_t*)(bytes + 1))[1];
        bool logOcEvents = bytes[9];

        emit onConfigChanged(logInterval, maxCurrent, logOcEvents);
    }
}

void HIDHandler::on_device_errored(QString error)
{
    std::cout << "ERROR: " << error.toStdString() << " | Aborting!" << std::endl;
    disconnectDevice();
}



void HIDReader::run()
{
    timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &HIDReader::on_timer_timeout);
    timer->start(CON_TIMEOUT);

    while(running) {
        if (isInterruptionRequested()) {
            break;
        }

        QByteArray data = hid->read(deviceId, 100);
        if (!data.isEmpty()) {
            timer->start(CON_TIMEOUT);
            emit on_data_received(data);
        }
        else {
            QString error = hid->error(deviceId);
            if (!error.isEmpty()) {
                emit on_device_errored(error);
                break;
            }
        }
    }

    timer->stop();
    disconnect(timer, &QTimer::timeout, this, &HIDReader::on_timer_timeout);
    delete timer;
}

void HIDReader::on_timer_timeout() //FIXME: doesn't fire
{
    running = false;
    emit on_device_errored("Connection timed out!");
}
