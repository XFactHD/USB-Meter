package usbplog.comm;

import com.serialpundit.core.SerialComException;
import com.serialpundit.serial.ISerialComDataListener;
import com.serialpundit.serial.SerialComManager;
import javafx.application.Platform;
import usbplog.Main;

import java.io.IOException;

public class SerialHandler implements ISerialComDataListener, ICommHandler
{
    private static final byte CMD_START = (byte) 0xA0;
    private static final byte CMD_STOP = (byte) 0xF0;
    private static final long SERIAL_TIMEOUT = 5000;
    private Thread commThread;
    private Deserializer deserializer;
    private boolean running = true;
    private boolean connectScheduled = false;
    private boolean disconnectScheduled = false;
    private boolean shutdownScheduled = false;

    private SerialComManager scm;
    private String port = "";
    private long portHandle = -1;
    private CommStatus commStatus = CommStatus.DISCONNECTED;
    private boolean answerReceived = false;
    private boolean errored = false;

    public void init()
    {
        commThread = new SerialCommThread();
        deserializer = new Deserializer(this);

        try
        {
            scm = new SerialComManager();
            listAllPorts();
        }
        catch (IOException e)
        {
            System.err.println("An error occurred while initializing serial communication");
            e.printStackTrace();
            return;
        }

        commThread.start();
    }

    public void listAllPorts()
    {
        try
        {
            String[] ports = scm.listAvailableComPorts();
            Main.INSTANCE.getDisplayManager().updatePortList(ports);
        }
        catch (SerialComException e)
        {
            System.out.println("An error occurred while listing serial ports");
            e.printStackTrace();
        }
    }

    public void scheduleConnect(String port)
    {
        this.port = port;
        connectScheduled = true;
    }

    public void scheduleDisconnect()
    {
        disconnectScheduled = true;
    }

    public void scheduleShutdown()
    {
        shutdownScheduled = true;
    }

    private void tryConnect(String port)
    {
        Platform.runLater(() -> Main.INSTANCE.getDisplayManager().setConnectionStatus(CommStatus.CONNECTING));
        commStatus = CommStatus.CONNECTING;
        threadSleep(500);

        try { portHandle = scm.openComPort(port, true, true, true); }
        catch (SerialComException e)
        {
            System.err.println("An error occurred while opening the serial port " + port);
            e.printStackTrace();
        }

        if (portHandle == -1)
        {
            Platform.runLater(() -> Main.INSTANCE.getDisplayManager().setConnectionStatus(CommStatus.DISCONNECTED));
            return;
        }

        try
        {
            scm.registerDataListener(portHandle, this);
            scm.configureComPortData(portHandle, SerialComManager.DATABITS.DB_8, SerialComManager.STOPBITS.SB_1, SerialComManager.PARITY.P_NONE, SerialComManager.BAUDRATE.B4000000, 0);
            scm.configureComPortControl(portHandle, SerialComManager.FLOWCONTROL.NONE, 'x', 'x', false, false);
        }
        catch (SerialComException e)
        {
            System.err.println("An error occurred while attaching the data listener to port " + port);
            e.printStackTrace();
            disconnect();
            return;
        }

        Platform.runLater(() -> Main.INSTANCE.getDisplayManager().setConnectionStatus(CommStatus.WAITING_RESPONSE));
        commStatus = CommStatus.WAITING_RESPONSE;

        sendByte(CMD_START);
        long startTime = System.currentTimeMillis();
        while (System.currentTimeMillis() - startTime < SERIAL_TIMEOUT && !answerReceived) { threadSleep(5); }

        if(!answerReceived) { disconnect(); commStatus = CommStatus.DISCONNECTED; }
        else { commStatus = CommStatus.CONNECTED; Platform.runLater(() -> Main.INSTANCE.getDisplayManager().setConnectionStatus(CommStatus.CONNECTED)); }
    }

    private void disconnect()
    {
        if (port.equals("") && portHandle == -1) { return; }

        try
        {
            if(!errored) { sendByte(CMD_STOP); }
            scm.unregisterDataListener(portHandle, this);
            scm.closeComPort(portHandle);
        }
        catch (SerialComException e)
        {
            System.err.println("An error occurred while closing the serial port " + port);
            e.printStackTrace();
        }

        portHandle = -1;
        port = "";
        Platform.runLater(() ->
        {
            Main.INSTANCE.getDisplayManager().setConnectionStatus(CommStatus.DISCONNECTED);
            Main.INSTANCE.getDisplayManager().signalDataEnd();
        });
        commStatus = CommStatus.DISCONNECTED;
        errored = false;
    }

    public void startAck()
    {
        answerReceived = true;
    }

    public void sendString(String data)
    {
        try
        {
            boolean result = scm.writeString(portHandle, data + '\n', 0);
            System.out.println(result);
        }
        catch (SerialComException e)
        {
            System.out.println("An error occurred while sending data to port " + port);
            e.printStackTrace();
        }
    }

    public void sendByte(byte data)
    {
        try
        {
            int result = scm.writeSingleByte(portHandle, data);
            //System.out.println(result);
        }
        catch (SerialComException e)
        {
            System.out.println("An error occurred while sending data to port " + port);
            e.printStackTrace();
        }
    }

    @Override
    public void onNewSerialDataAvailable(byte[] bytes)
    {
        //System.out.print(commStatus.toString() + " { ");
        //System.out.print(bytesToHex(bytes));
        //System.out.println(" }");

        deserializer.deserialize(bytes);
    }

    private static final char[] HEX_ARRAY = "0123456789ABCDEF".toCharArray();
    public static String bytesToHex(byte[] bytes) {
        char[] hexChars = new char[bytes.length * 3 - 1];
        for (int j = 0; j < bytes.length; j++) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 3] = HEX_ARRAY[v >>> 4];
            hexChars[j * 3 + 1] = HEX_ARRAY[v & 0x0F];
            if (j < bytes.length - 1) { hexChars[j * 3 + 2] = ' '; }
        }
        return new String(hexChars);
    }

    @Override
    public void onDataListenerError(int error)
    {
        errored = true;
        disconnect();
    }

    private static void threadSleep(int millis)
    {
        try { Thread.sleep(millis); }
        catch (InterruptedException ignore) {}
    }

    private class SerialCommThread extends Thread
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
                    tryConnect(port);
                }
                else if (disconnectScheduled)
                {
                    disconnectScheduled = false;
                    disconnect();
                }
                else if (shutdownScheduled)
                {
                    if(commStatus != CommStatus.DISCONNECTED) { disconnect(); }
                    running = false;
                    System.out.println("Thread stopped");
                }

                threadSleep(10);
            }
        }
    }
}