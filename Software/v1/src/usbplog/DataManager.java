package usbplog;

import javafx.application.Platform;

import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;

public class DataManager
{
    private boolean logging = false;
    private File logFile = null;
    private PrintWriter fileWriter = null;
    private int dataPoints = 0;
    private int byteCount = 0;

    public void init()
    {

    }

    public void onDataReceived(long timestamp, double voltage, double current, double power)
    {
        if(!logging) { return; }

        String data = timestamp + "ms," + voltage + "V," + current + "mA," + power + "mW";
        fileWriter.println(data);

        dataPoints++;
        byteCount += data.length() + 1; //+1 for the newline

        Platform.runLater(() -> Main.INSTANCE.getDisplayManager().updateLogInfo(dataPoints, byteCount));
    }

    public void setLogFile(File logFile)
    {
        this.logFile = logFile;
    }

    public void startLogging()
    {
        try
        {
            fileWriter = new PrintWriter(logFile);
        }
        catch (IOException e)
        {
            System.err.println("An error occurred while creating or opening the log file");
            e.printStackTrace();
            return;
        }
        logging = true;
        Main.INSTANCE.getDisplayManager().setLoggingStatus(true);

        String header = "Timestamp,Voltage,Current,Power";
        byteCount += header.length() + 1; //+1 for the newline
        Main.INSTANCE.getDisplayManager().updateLogInfo(0, byteCount);
        fileWriter.println(header);
    }

    public void stopLogging()
    {
        logging = false;

        fileWriter.flush();
        fileWriter.close();

        dataPoints = 0;
        byteCount = 0;
        Main.INSTANCE.getDisplayManager().updateLogInfo(0, 0);
        Main.INSTANCE.getDisplayManager().setLoggingStatus(false);
    }
}