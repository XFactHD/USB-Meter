package usbplog;

import javafx.application.Application;
import javafx.scene.Scene;
import javafx.scene.layout.GridPane;
import javafx.stage.Stage;
import javafx.stage.Window;
import usbplog.comm.HIDHandler;
import usbplog.comm.ICommHandler;
import usbplog.comm.SerialHandler;
import usbplog.config.ConfigHandler;
import usbplog.display.DisplayManager;
import usbplog.display.LocaleManager;

import java.util.Locale;

public class Main extends Application
{
    public static Main INSTANCE;

    private Stage primaryStage;
    private Window ownerWindow;
    private GridPane root;
    private ConfigHandler configHandler = new ConfigHandler();
    private DisplayManager displayManager = new DisplayManager();
    private DataManager dataManager = new DataManager();
    private ICommHandler commHandler;

    public static void main(String[] args) { launch(args); }

    @Override
    public void start(Stage primaryStage)
    {
        INSTANCE = this;

        GridPane root = new GridPane();
        this.root = root;

        primaryStage.setTitle("USB-PowerLogger");
        primaryStage.setScene(new Scene(root, 1930, 1256));

        configHandler.init();
        buildStyle();
        Locale.setDefault(configHandler.getLocale());
        LocaleManager.parseLangFile(configHandler.getLocale());
        displayManager.init(root);
        commHandler = configHandler.getUseHid() ? new HIDHandler() : new SerialHandler();
        commHandler.init();
        dataManager.init();

        primaryStage.setOnCloseRequest(event -> shutdown());

        primaryStage.show();
        this.primaryStage = primaryStage;
        ownerWindow = primaryStage.getScene().getWindow();
    }

    public Stage getPrimaryStage() { return primaryStage; }

    public Window getOwnerWindow() { return ownerWindow; }

    public ConfigHandler getConfigHandler() { return configHandler; }

    public DisplayManager getDisplayManager() { return displayManager; }

    public DataManager getDataManager() { return dataManager; }

    public ICommHandler getCommHandler() { return commHandler; }

    public void setDarkMode(boolean dark)
    {
        configHandler.setDarkMode(dark);
        buildStyle();
    }

    //TODO: save to config instead of setting it directly
    public void setLanguage(Locale locale)
    {
        Locale.setDefault(locale);
        configHandler.setLocale(locale);
    }

    private void buildStyle()
    {
        String style = "-fx-font: bold 18px Monospaced;";
        if (configHandler.getDarkMode()) { style += "-fx-base: rgba(60, 63, 65, 255);"; }

        root.setStyle(style);
    }

    public void shutdown()
    {
        displayManager.shutdown();
        commHandler.scheduleShutdown();
        configHandler.shutdown();
    }
}