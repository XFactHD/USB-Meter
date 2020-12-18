package usbplog.display;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.Locale;

public class LocaleManager
{
    private static final String LANG_PATH = "/resources/lang/";
    private static HashMap<String, String> langMap = new HashMap<>();

    public static void parseLangFile(Locale locale)
    {
        String name = toFileName(locale);
        InputStream in = LocaleManager.class.getResourceAsStream(LANG_PATH + name);
        if (in == null) { in = LocaleManager.class.getResourceAsStream(LANG_PATH + "en_US.lang"); }

        BufferedReader reader = new BufferedReader(new InputStreamReader(in));
        reader.lines().forEach(s ->
        {
            if(!s.isEmpty() && s.contains("="))
            {
                String key = s.substring(0, s.indexOf('='));
                String val = s.substring(s.indexOf('=') + 1);
                langMap.put(key, val);
            }
        });
        try
        {
            reader.close();
        }
        catch (IOException ignored) { }
    }

    public static String getTranslation(String key)
    {
        return langMap.getOrDefault(key, key);
    }

    public static boolean isLangAvailable(Locale locale)
    {
        return LocaleManager.class.getResource(LANG_PATH + toFileName(locale)) != null;
    }

    private static String toFileName(Locale locale)
    {
        String name = locale.getLanguage();
        if (!locale.getCountry().isEmpty()) { name += "_" + locale.getCountry(); }
        return name + ".lang";
    }
}