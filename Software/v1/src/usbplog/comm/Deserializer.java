package usbplog.comm;

import javafx.application.Platform;
import usbplog.Main;

import java.nio.ByteBuffer;
import java.util.Arrays;

public class Deserializer
{
    private static final byte CMD_START_ACK = (byte) 0xA5;
    private static final byte CMD_CFG = (byte) 0xB0;
    private static final byte CMD_CFG_ACK = (byte) 0xB5;
    private static final byte CMD_DATA_POINT = (byte) 0xC0;
    private static final byte CMD_DATA_ACK = (byte) 0xC5;

    private ICommHandler commHandler;

    public Deserializer (ICommHandler handler)
    {
        commHandler = handler;
    }

    public void deserialize(byte[] data)
    {
        switch (data[0])
        {
            case CMD_START_ACK:
            {
                commHandler.startAck();
                Platform.runLater(() -> Main.INSTANCE.getDisplayManager().signalDataStart());

                //Make sure the config gets received in case it gets received together with the start ack (only applicable to serial)
                if(!Main.INSTANCE.getConfigHandler().getUseHid() && data.length > 1) { deserialize(Arrays.copyOfRange(data, 1, data.length)); }
                break;
            }
            case CMD_CFG:
            {
                int logInterval = bytesToInt(new byte[] { data[ 4], data[ 3], data[ 2], data[ 1] });
                int maxCurrent =  bytesToInt(new byte[] { data[ 8], data[ 7], data[ 6], data[ 5] });
                boolean logOCEvents = data[9] != 0;
                Platform.runLater(() -> Main.INSTANCE.getDisplayManager().configureDisplay(logInterval, maxCurrent, logOCEvents));
                commHandler.sendByte(CMD_CFG_ACK);
                //System.out.println("Interval: " + logInterval + ", I Max: " + maxCurrent + ", Log OC: " + logOCEvents);
                break;
            }
            case CMD_DATA_POINT:
            {
                long timestamp = bytesToInt(new byte[] { data[ 4], data[ 3], data[ 2], data[ 1] });
                float voltage = bytesToFloat(new byte[] { data[ 8], data[ 7], data[ 6], data[ 5] });
                float current = bytesToFloat(new byte[] { data[12], data[11], data[10], data[ 9] });
                float power =   bytesToFloat(new byte[] { data[16], data[15], data[14], data[13] });

                //System.out.println(timestamp);

                Platform.runLater(() -> Main.INSTANCE.getDisplayManager().addDataPoint(timestamp, voltage, current, power));
                Main.INSTANCE.getDataManager().onDataReceived(timestamp, voltage, current, power);

                commHandler.sendByte(CMD_DATA_ACK);
            }
        }
    }

    public int bytesToInt(byte[] bytes)
    {
        ByteBuffer buffer = ByteBuffer.allocate(Integer.BYTES);
        buffer.put(bytes);
        buffer.flip(); //need flip
        return buffer.getInt();
    }

    public float bytesToFloat(byte[] bytes)
    {
        ByteBuffer buffer = ByteBuffer.allocate(Float.BYTES);
        buffer.put(bytes);
        buffer.flip();
        return buffer.getFloat();
    }
}