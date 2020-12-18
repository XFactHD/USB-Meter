package usbplog.comm;

public interface ICommHandler
{
    void init();

    default void listAllPorts() {} // Defaults to no-op for HID

    void scheduleConnect(String data);

    void scheduleDisconnect();

    void scheduleShutdown();

    void startAck();

    void sendString(String data);

    void sendByte(byte data);


}