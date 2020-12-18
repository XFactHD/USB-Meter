package usbplog.comm;

import com.serialpundit.core.SerialComException;
import com.serialpundit.hid.IHIDInputReportListener;
import com.serialpundit.hid.SerialComRawHID;
import com.serialpundit.usb.ISerialComUSBHotPlugListener;
import com.serialpundit.usb.SerialComUSB;
import com.serialpundit.usb.SerialComUSBHID;
import com.serialpundit.usb.SerialComUSBHIDdevice;
import javafx.application.Platform;
import usbplog.Main;

import java.util.Arrays;

public class HIDHandler implements IHIDInputReportListener, ISerialComUSBHotPlugListener, ICommHandler
{
    private static final byte CMD_START = (byte) 0xA0;
    private static final byte CMD_STOP = (byte) 0xF0;
    private static final long HID_TIMEOUT = 5000;
    private static final int BUFFER_SIZE = 32;

    private final int PRODUCT_VID = 0xF055;//0x239A; //TODO: remove serial number settings
    private final int PRODUCT_PID = 0x0000;//0x8031;

    private String productSerialNum;

    @SuppressWarnings("FieldCanBeLocal")
    private Thread commThread;
    private Deserializer deserializer;
    private SerialComUSB comUSB;
    private SerialComRawHID rawHID;
    private SerialComUSBHID usbHID;

    private boolean running = true;
    private boolean deviceAvailable = false;
    private boolean connectScheduled = false;
    private boolean disconnectScheduled = false;
    private boolean shutdownScheduled = false;
    private boolean answerReceived = false;
    private CommStatus commStatus = CommStatus.DISCONNECTED;
    @SuppressWarnings({ "FieldCanBeLocal", "unused" })
    private boolean errored = false;

    private int hotplugHandle = -1;
    private String hidDeviceNode;
    private long hidDeviceHandle = -1;

    private byte[] inputReportBuffer = new byte[BUFFER_SIZE];
    private byte[] outputReportBuffer = new byte[BUFFER_SIZE];

    @Override
    public void init()
    {
        commThread = new HIDCommThread();
        deserializer = new Deserializer(this);
        productSerialNum = Main.INSTANCE.getConfigHandler().getProductSerialNumber();

        try
        {
            comUSB = new SerialComUSB(null, null);
            rawHID = new SerialComRawHID(null, null);
            usbHID = comUSB.getUSBHIDTransportInstance();

            hotplugHandle = comUSB.registerUSBHotPlugEventListener(this, PRODUCT_VID, PRODUCT_PID, null);
            deviceAvailable = comUSB.isUSBDevConnected(PRODUCT_VID, PRODUCT_PID, null);
            if (deviceAvailable) { Main.INSTANCE.getDisplayManager().updateHIDAvailable(deviceAvailable); }
        }
        catch (Exception e)
        {
            System.err.println("An error occured while initializing HID communication!");
            e.printStackTrace();
        }

        commThread.start();
    }

    @Override
    public void scheduleConnect(String data) { connectScheduled = true; }

    @Override
    public void scheduleDisconnect() { disconnectScheduled = true; }

    @Override
    public void scheduleShutdown() { shutdownScheduled = true; }

    @Override
    public void startAck() { answerReceived = true; }

    @Override
    public void sendByte(byte data)
    {
        try
        {
            Arrays.fill(outputReportBuffer, (byte) 0);
            outputReportBuffer[0] = data;
            int sent = rawHID.writeOutputReportR(hidDeviceHandle, (byte) -1, outputReportBuffer);
            //System.out.println("Bytes sent: " + sent);
        }
        catch (SerialComException e)
        {
            System.err.println("An error ocurred while sending data to HID device!");
            e.printStackTrace();
        }
    }

    @Override
    public void sendString(String data)
    {
        try
        {
            Arrays.fill(outputReportBuffer, (byte) 0);
            System.arraycopy(data.getBytes(), 0, outputReportBuffer, 0, Math.min(data.length(), BUFFER_SIZE));
            rawHID.writeOutputReportR(hidDeviceHandle, (byte) -1, outputReportBuffer);

            if (data.length() > BUFFER_SIZE) { sendString(data.substring(32)); }
        }
        catch (SerialComException e)
        {
            System.err.println("An error ocurred while sending data to HID device!");
            e.printStackTrace();
        }
    }

    private void tryConnect()
    {
        if (!deviceAvailable || hidDeviceHandle != -1) { return; }

        setCommStatus(CommStatus.CONNECTING);
        threadSleep(500);

        try
        {
            SerialComUSBHIDdevice[] devices = usbHID.listUSBHIDdevicesWithInfo(PRODUCT_VID);
            for (SerialComUSBHIDdevice device : devices)
            {
                if (device.getProductID() == PRODUCT_PID)
                {
                    hidDeviceNode = device.getDeviceNode();
                    hidDeviceHandle = rawHID.openHidDeviceR(hidDeviceNode, false);
                    rawHID.registerInputReportListener(hidDeviceHandle, this, inputReportBuffer);
                    break;
                }
            }

            if (hidDeviceHandle == -1) { setCommStatus(CommStatus.DISCONNECTED); return; }
        }
        catch (SerialComException e)
        {
            System.err.println("An error occured while opening HID device!");
            e.printStackTrace();
            setCommStatus(CommStatus.DISCONNECTED);
            return;
        }

        threadSleep(500);

        sendByte(CMD_START);
        long startTime = System.currentTimeMillis();
        while (System.currentTimeMillis() - startTime < HID_TIMEOUT && !answerReceived) { threadSleep(5); }

        if(!answerReceived) { disconnect(); setCommStatus(CommStatus.DISCONNECTED); }
        else { setCommStatus(CommStatus.CONNECTED); }
    }

    private void disconnect()
    {
        if (hidDeviceHandle == -1) { return; }

        try
        {
            rawHID.unregisterInputReportListener(this);
            rawHID.closeHidDeviceR(hidDeviceHandle);
        }
        catch (SerialComException e)
        {
            System.err.println("An error ocurred while disconnecting HID device!");
            e.printStackTrace();
        }

        hidDeviceHandle = -1;
        setCommStatus(CommStatus.DISCONNECTED);
        Platform.runLater(() -> Main.INSTANCE.getDisplayManager().signalDataEnd());
    }

    private void setCommStatus(CommStatus status)
    {
        commStatus = status;
        Platform.runLater(() ->
        {
            Main.INSTANCE.getDisplayManager().setConnectionStatus(status);
            Main.INSTANCE.getDisplayManager().updateHIDAvailable(deviceAvailable);
        });
    }

    //Event methods
    @Override
    public void onNewInputReportAvailable(int size, byte[] buffer)
    {
        //System.out.println("Bytes received: " + size + ", Data: " + SerialHandler.bytesToHex(buffer));
        deserializer.deserialize(buffer);
    }

    @Override
    public void onNewInputReportAvailableError(Exception e)
    {
        errored = true;
        disconnect();
    }

    @Override
    public void onUSBHotPlugEvent(int event, int vid, int pid, String serialNum)
    {
        deviceAvailable = event == SerialComUSB.DEV_ADDED;
        Platform.runLater(() -> Main.INSTANCE.getDisplayManager().updateHIDAvailable(deviceAvailable));

        if (!deviceAvailable) { scheduleDisconnect(); }

        System.out.println("HID device " + (event == SerialComUSB.DEV_ADDED ? "" : "dis") + "connected!");
    }

    //Thread methods
    private static void threadSleep(int millis)
    {
        try { Thread.sleep(millis); }
        catch (InterruptedException ignore) {}
    }

    private class HIDCommThread extends Thread
    {
        @Override
        public void run()
        {
            System.out.println("Thread started");
            while (running)
            {
                if (connectScheduled)
                {
                    connectScheduled = false;
                    tryConnect();
                }
                else if (disconnectScheduled)
                {
                    disconnectScheduled = false;
                    disconnect();
                }
                else if (shutdownScheduled)
                {
                    if(commStatus != CommStatus.DISCONNECTED) { disconnect(); }
                    try { comUSB.unregisterUSBHotPlugEventListener(hotplugHandle); }
                    catch (SerialComException e)
                    {
                        System.err.println("An error ocurred while removing hot plug listener");
                        e.printStackTrace();
                    }
                    running = false;
                    System.out.println("Thread stopped");
                }

                threadSleep(10);
            }
        }
    }
}