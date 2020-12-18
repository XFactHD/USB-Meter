package usbplog.config;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

public class ConfigHandler
{
    private File cfgFile;

    private Locale locale;
    private boolean darkMode;
    private boolean useHid;
    private String serialNumber;

    public void init()
    {
        Map<String, String> values = new HashMap<>();
        cfgFile = new File(System.getProperty("user.home") + "\\usb-plog.cfg");

        try
        {
            boolean exists = cfgFile.exists();
            if (!exists && !cfgFile.createNewFile()) { throw new IOException("Failed to create config file!"); }
            else if (exists) //If the file didn't exist, we don't need to try to read anything
            {
                BufferedReader reader = new BufferedReader(new InputStreamReader(new FileInputStream(cfgFile), StandardCharsets.UTF_8));
                reader.lines().forEach(s ->
                {
                    if (!s.isEmpty() && s.contains("="))
                    {
                        String key = s.substring(0, s.indexOf('='));
                        String val = s.substring(s.indexOf('=') + 1);
                        values.put(key, val);
                    }
                });
                reader.close();
            }
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }

        locale = Locale.forLanguageTag(values.getOrDefault("locale", "en-US"));
        darkMode = values.getOrDefault("dark_mode", "false").equals("true");
        useHid = values.getOrDefault("use_hid", "false").equals("true");
        serialNumber = values.getOrDefault("serial_number", "");
    }

    public void shutdown()
    {
        PrintWriter fileWriter;
        try { fileWriter = new PrintWriter(cfgFile); }
        catch (IOException e)
        {
            System.err.println("An error occurred while creating or opening the config file");
            e.printStackTrace();
            return;
        }

        fileWriter.println("locale=" + locale.toLanguageTag());
        fileWriter.println("dark_mode=" + (darkMode ? "true" : "false"));
        fileWriter.println("use_hid=" + (useHid ? "true" : "false"));
        fileWriter.println("serial_number=" + serialNumber);

        fileWriter.flush();
        fileWriter.close();
    }


    //Settings getters and setters
    public Locale getLocale() { return locale; }

    public void setLocale(Locale locale) { this.locale = locale; }

    public boolean getDarkMode() { return darkMode; }

    public void setDarkMode(boolean darkMode) { this.darkMode = darkMode; }

    public boolean getUseHid() { return useHid; }

    public void setUseHid(boolean useHid) { this.useHid = useHid; }

    public String getProductSerialNumber() { return serialNumber; }

    public void setProductSerialNumber(String serialNumber) { this.serialNumber = serialNumber; }
}