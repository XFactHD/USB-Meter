package usbplog.display;

import javafx.application.Platform;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.chart.LineChart;
import javafx.scene.chart.NumberAxis;
import javafx.scene.chart.XYChart;
import javafx.scene.control.*;
import javafx.scene.layout.*;
import javafx.scene.paint.Color;
import javafx.stage.FileChooser;
import javafx.util.Pair;
import usbplog.comm.CommStatus;
import usbplog.Main;

import java.io.File;
import java.util.*;

public class DisplayManager
{
    //Constants
    private final int MAX_TIME_WIDTH = 5000;
    private final double MAX_VOLTAGE = 5.5;
    private final int MAX_CURRENT = 3000;

    //Misc values
    private CommStatus connStatus = CommStatus.DISCONNECTED;
    private int logInterval = 0;
    private int maxItems = 0;
    @SuppressWarnings({ "FieldCanBeLocal", "unused" })
    private double currentLimit = MAX_CURRENT;
    boolean currentLimitPostponed = false;
    private double minVoltage = 10;
    private double maxVoltage = 0;
    private double minCurrent = MAX_CURRENT;
    private double maxCurrent = 0;
    private double minPower = 20000;
    private double maxPower = 0;
    private long lastTimestamp = 0;

    //Voltage graph
    private NumberAxis xAxisVoltage;
    private NumberAxis yAxisVoltage;
    private XYChart.Series<Number, Number> seriesVoltage;
    private LineChart<Number, Number> chartVoltage;

    //Current graph
    private NumberAxis xAxisCurrent;
    private NumberAxis yAxisCurrent;
    private XYChart.Series<Number, Number> seriesCurrent;
    private LineChart<Number, Number> chartCurrent;

    //Power graph
    private NumberAxis xAxisPower;
    private NumberAxis yAxisPower;
    private XYChart.Series<Number, Number> seriesPower;
    @SuppressWarnings("FieldCanBeLocal")
    private LineChart<Number, Number> chartPower;

    //Voltage info
    private Label voltageNow;
    private Label voltageMin;
    private Label voltageMax;

    //Current info
    private Label currentNow;
    private Label currentMin;
    private Label currentMax;

    //Power info
    private Label powerNow;
    private Label powerMin;
    private Label powerMax;

    //Big live display
    private Label voltageLive;
    private Label currentLive;
    private Label powerLive;

    //Logger info
    TextField fileName;
    Label dataPointCount;
    Label fileSize;
    private Button startLogging;

    //Comm info
    private Label connected;
    private ComboBox<String> portList;
    private Button scanPorts;
    private Button connect;

    //Misc buttons
    Button resetAllDisplays;
    Button resetGraphs;

    //Init methods
    public void init(GridPane root)
    {
        GridPane mainPane = createGridPane(0, 0);
        mainPane.setPrefSize(1930, 1218);
        mainPane.setBorder(new Border(new BorderStroke(Color.BLACK, BorderStrokeStyle.SOLID, CornerRadii.EMPTY, BorderStroke.THICK)));
        root.add(mainPane, 0, 1);

        GridPane dataPane = createGridPane(0,0);
        dataPane.setBorder(new Border(new BorderStroke(Color.BLACK, BorderStrokeStyle.SOLID, CornerRadii.EMPTY, BorderStroke.MEDIUM)));
        mainPane.add(dataPane, 0, 0);

        GridPane voltagePane = createGridPane(25, 25);
        voltagePane.setBorder(new Border(new BorderStroke(Color.BLACK, BorderStrokeStyle.SOLID, CornerRadii.EMPTY, BorderStroke.THIN)));
        dataPane.add(voltagePane, 0, 0);
        GridPane currentPane = createGridPane(25, 25);
        currentPane.setBorder(new Border(new BorderStroke(Color.BLACK, BorderStrokeStyle.SOLID, CornerRadii.EMPTY, BorderStroke.THIN)));
        dataPane.add(currentPane, 0, 1);
        GridPane powerPane   = createGridPane(25, 25);
        powerPane.setBorder(new Border(new BorderStroke(Color.BLACK, BorderStrokeStyle.SOLID, CornerRadii.EMPTY, BorderStroke.THIN)));
        dataPane.add(powerPane, 0, 2);

        GridPane rightPane = createGridPane(0, 0);
        rightPane.setBorder(new Border(new BorderStroke(Color.BLACK, BorderStrokeStyle.SOLID, CornerRadii.EMPTY, BorderStroke.MEDIUM)));
        mainPane.add(rightPane, 1, 0);

        GridPane livePane = createGridPane(25, 25);
        livePane.setBorder(new Border(new BorderStroke(Color.BLACK, BorderStrokeStyle.SOLID, CornerRadii.EMPTY, BorderStroke.THIN)));
        rightPane.add(livePane, 0, 0);
        GridPane logPane = createGridPane(25, 5);
        logPane.setBorder(new Border(new BorderStroke(Color.BLACK, BorderStrokeStyle.SOLID, CornerRadii.EMPTY, BorderStroke.THIN)));
        rightPane.add(logPane, 0, 1);
        GridPane commPane = createGridPane(25, 5);
        commPane.setBorder(new Border(new BorderStroke(Color.BLACK, BorderStrokeStyle.SOLID, CornerRadii.EMPTY, BorderStroke.THIN)));
        rightPane.add(commPane, 0, 2);
        GridPane buttonPane = createGridPane(25, 5);
        buttonPane.setBorder(new Border(new BorderStroke(Color.BLACK, BorderStrokeStyle.SOLID, CornerRadii.EMPTY, BorderStroke.THIN)));
        rightPane.add(buttonPane, 0, 3);

        initGraphs(voltagePane, currentPane, powerPane);
        initMenuBar(root);
        initInfoDisplay(voltagePane, currentPane, powerPane);
        initLiveDisplay(livePane);
        initLogInfoDisplay(logPane);
        initCommInfoDisplay(commPane);
        initMiscButtons(buttonPane);
    }

    private void initGraphs(GridPane voltagePane, GridPane currentPane, GridPane powerPane)
    {
        xAxisVoltage = new NumberAxis(LocaleManager.getTranslation("axis.time.name"), 0, MAX_TIME_WIDTH, 250);
        xAxisVoltage.setForceZeroInRange(false);
        yAxisVoltage = new NumberAxis(LocaleManager.getTranslation("axis.voltage.name"), 0, MAX_VOLTAGE, .5D);
        seriesVoltage = new XYChart.Series<>();
        chartVoltage = new LineChart<>(xAxisVoltage, yAxisVoltage);
        chartVoltage.setCreateSymbols(false);
        chartVoltage.setLegendVisible(false);
        chartVoltage.setAnimated(false);
        chartVoltage.getData().add(seriesVoltage);
        chartVoltage.setPrefWidth(800);
        chartVoltage.setPrefHeight(400);
        voltagePane.add(chartVoltage, 0, 0);

        xAxisCurrent = new NumberAxis(LocaleManager.getTranslation("axis.time.name"), 0, MAX_TIME_WIDTH, 250);
        xAxisCurrent.setForceZeroInRange(false);
        yAxisCurrent = new NumberAxis(LocaleManager.getTranslation("axis.current.name"), 0, MAX_CURRENT, 250);
        seriesCurrent = new XYChart.Series<>();
        chartCurrent = new LineChart<>(xAxisCurrent, yAxisCurrent);
        chartCurrent.setCreateSymbols(false);
        chartCurrent.setLegendVisible(false);
        chartCurrent.setAnimated(false);
        chartCurrent.getData().add(seriesCurrent);
        chartCurrent.setPrefWidth(800);
        chartCurrent.setPrefHeight(400);
        currentPane.add(chartCurrent, 0, 0);

        xAxisPower = new NumberAxis(LocaleManager.getTranslation("axis.time.name"), 0, MAX_TIME_WIDTH, 250);
        xAxisPower.setForceZeroInRange(false);
        yAxisPower = new NumberAxis(LocaleManager.getTranslation("axis.power.name"), 0, MAX_VOLTAGE * MAX_CURRENT, 1375);
        seriesPower = new XYChart.Series<>();
        chartPower = new LineChart<>(xAxisPower, yAxisPower);
        chartPower.setCreateSymbols(false);
        chartPower.setLegendVisible(false);
        chartPower.setAnimated(false);
        chartPower.getData().add(seriesPower);
        chartPower.setPrefWidth(800);
        chartPower.setPrefHeight(400);
        powerPane.add(chartPower, 0, 0);

        yAxisPower.widthProperty().addListener((observable, oldValue, newValue) ->
        {
            //Set the width to the new value
            yAxisVoltage.setPrefWidth(newValue.doubleValue());
            yAxisCurrent.setPrefWidth(newValue.doubleValue());

            //Force a geometry update
            chartVoltage.applyCss();
            chartCurrent.applyCss();
            chartVoltage.layout();
            chartCurrent.layout();
        });

        resetGraphs();
    }

    private void initMenuBar(GridPane pane)
    {
        MenuBar menuBar = new MenuBar();

        Menu fileMenu = new Menu(LocaleManager.getTranslation("menu.file.name"));
        menuBar.getMenus().add(fileMenu);

        Menu settingsMenu = new Menu(LocaleManager.getTranslation("menu.settings.name"));
        fileMenu.getItems().add(settingsMenu);

        initSettingsMenu(settingsMenu);

        MenuItem closeItem = new MenuItem(LocaleManager.getTranslation("menu.item.close.name"));
        closeItem.setOnAction(event ->
        {
            event.consume();
            Main.INSTANCE.shutdown();
            Main.INSTANCE.getPrimaryStage().close(); //FIXME: doesn't call the on close handler
        });
        fileMenu.getItems().add(closeItem);

        pane.add(menuBar, 0, 0);
    }

    private void initSettingsMenu(Menu settingsMenu)
    {
        RadioMenuItem darkModeItem = new RadioMenuItem(LocaleManager.getTranslation("menu.item.dark.name"));
        darkModeItem.setSelected(Main.INSTANCE.getConfigHandler().getDarkMode());
        darkModeItem.setOnAction(event ->
        {
            event.consume();
            Main.INSTANCE.setDarkMode(darkModeItem.isSelected());
        });
        settingsMenu.getItems().add(darkModeItem);

        Menu langMenu = new Menu(LocaleManager.getTranslation("menu.lang.name"));
        settingsMenu.getItems().add(langMenu);

        Locale[] locales = Locale.getAvailableLocales();
        Arrays.sort(locales, Comparator.comparing(o -> o.getDisplayName(o)));
        ToggleGroup localeGroup = new ToggleGroup();
        for (Locale locale : locales)
        {
            if (!LocaleManager.isLangAvailable(locale)) { continue; }

            RadioMenuItem langItem = new RadioMenuItem(locale.getDisplayName(locale));
            langItem.setToggleGroup(localeGroup);
            if (Main.INSTANCE.getConfigHandler().getLocale() == locale) { langItem.setSelected(true); }
            langItem.setOnAction(event ->
            {
                event.consume();
                if (langItem.isSelected()) { Main.INSTANCE.setLanguage(locale); }
            });
            langMenu.getItems().add(langItem);
        }

        MenuItem hidItem = new MenuItem(LocaleManager.getTranslation("menu.item.hid.name"));
        hidItem.setOnAction(event ->
        {
            event.consume();
            openHIDSettings();
        });
        settingsMenu.getItems().add(hidItem);
    }

    private void initInfoDisplay(GridPane voltagePane, GridPane currentPane, GridPane powerPane)
    {
        //Voltage info
        GridPane voltageInfoPane = createGridPane(5, 5);
        voltageInfoPane.setAlignment(Pos.CENTER);
        voltageInfoPane.setPadding(new Insets(30));

        Label voltageNowLabel = new Label(LocaleManager.getTranslation("label.voltage.now.name"));
        voltageNowLabel.setPrefWidth(184);
        voltageNow = new Label("0.00V");
        voltageNow.setPrefSize(100, 32);
        voltageNow.setAlignment(Pos.CENTER_RIGHT);
        voltageNow.setPadding(new Insets(0, 5, 0, 0));
        voltageNow.setBorder(new Border(new BorderStroke(Color.BLACK, BorderStrokeStyle.SOLID, CornerRadii.EMPTY, BorderStroke.THIN)));

        Label voltageMinLabel = new Label(LocaleManager.getTranslation("label.voltage.min.name"));
        voltageMinLabel.setPrefWidth(184);
        voltageMin = new Label("0.00V");
        voltageMin.setPrefSize(100, 32);
        voltageMin.setAlignment(Pos.CENTER_RIGHT);
        voltageMin.setPadding(new Insets(0, 5, 0, 0));
        voltageMin.setBorder(new Border(new BorderStroke(Color.BLACK, BorderStrokeStyle.SOLID, CornerRadii.EMPTY, BorderStroke.THIN)));

        Label voltageMaxLabel = new Label(LocaleManager.getTranslation("label.voltage.max.name"));
        voltageMaxLabel.setPrefWidth(184);
        voltageMax = new Label("0.00V");
        voltageMax.setPrefSize(100, 32);
        voltageMax.setAlignment(Pos.CENTER_RIGHT);
        voltageMax.setPadding(new Insets(0, 5, 0, 0));
        voltageMax.setBorder(new Border(new BorderStroke(Color.BLACK, BorderStrokeStyle.SOLID, CornerRadii.EMPTY, BorderStroke.THIN)));
        
        voltageInfoPane.addColumn(0, voltageNowLabel, voltageMinLabel, voltageMaxLabel);
        voltageInfoPane.addColumn(1, voltageNow, voltageMin, voltageMax);

        Button resetVoltage = new Button(LocaleManager.getTranslation("button.reset.minmax.name"));
        resetVoltage.setOnMouseClicked(event -> resetMinMaxVoltage());
        voltageInfoPane.add(resetVoltage, 0, 5, 2, 1);

        voltagePane.add(voltageInfoPane, 1, 0);

        //Current info
        GridPane currentInfoPane = createGridPane(5, 5);
        currentInfoPane.setAlignment(Pos.CENTER);
        currentInfoPane.setPadding(new Insets(30));

        Label currentNowLabel = new Label(LocaleManager.getTranslation("label.current.now.name"));
        currentNowLabel.setPrefWidth(184);
        currentNow = new Label("0.00mA");
        currentNow.setPrefSize(100, 32);
        currentNow.setAlignment(Pos.CENTER_RIGHT);
        currentNow.setPadding(new Insets(0, 5, 0, 0));
        currentNow.setBorder(new Border(new BorderStroke(Color.BLACK, BorderStrokeStyle.SOLID, CornerRadii.EMPTY, BorderStroke.THIN)));

        Label currentMinLabel = new Label(LocaleManager.getTranslation("label.current.min.name"));
        currentMinLabel.setPrefWidth(184);
        currentMin = new Label("0.00mA");
        currentMin.setPrefSize(100, 32);
        currentMin.setAlignment(Pos.CENTER_RIGHT);
        currentMin.setPadding(new Insets(0, 5, 0, 0));
        currentMin.setBorder(new Border(new BorderStroke(Color.BLACK, BorderStrokeStyle.SOLID, CornerRadii.EMPTY, BorderStroke.THIN)));

        Label currentMaxLabel = new Label(LocaleManager.getTranslation("label.current.max.name"));
        currentMaxLabel.setPrefWidth(184);
        currentMax = new Label("0.00mA");
        currentMax.setPrefSize(100, 32);
        currentMax.setAlignment(Pos.CENTER_RIGHT);
        currentMax.setPadding(new Insets(0, 5, 0, 0));
        currentMax.setBorder(new Border(new BorderStroke(Color.BLACK, BorderStrokeStyle.SOLID, CornerRadii.EMPTY, BorderStroke.THIN)));

        currentInfoPane.addColumn(0, currentNowLabel, currentMinLabel, currentMaxLabel);
        currentInfoPane.addColumn(1, currentNow, currentMin, currentMax);

        Button resetCurrent = new Button(LocaleManager.getTranslation("button.reset.minmax.name"));
        resetCurrent.setOnMouseClicked(event -> resetMinMaxCurrent());
        currentInfoPane.add(resetCurrent, 0, 5, 2, 1);

        currentPane.add(currentInfoPane, 1, 0);

        //Power info
        GridPane powerInfoPane = createGridPane(5, 5);
        powerInfoPane.setAlignment(Pos.CENTER);
        powerInfoPane.setPadding(new Insets(30));

        Label powerNowLabel = new Label(LocaleManager.getTranslation("label.power.now.name"));
        powerNowLabel.setPrefWidth(184);
        powerNow = new Label("0.000W");
        powerNow.setPrefSize(100, 32);
        powerNow.setAlignment(Pos.CENTER_RIGHT);
        powerNow.setPadding(new Insets(0, 5, 0, 0));
        powerNow.setBorder(new Border(new BorderStroke(Color.BLACK, BorderStrokeStyle.SOLID, CornerRadii.EMPTY, BorderStroke.THIN)));

        Label powerMinLabel = new Label(LocaleManager.getTranslation("label.power.min.name"));
        powerMinLabel.setPrefWidth(184);
        powerMin = new Label("0.000W");
        powerMin.setPrefSize(100, 32);
        powerMin.setAlignment(Pos.CENTER_RIGHT);
        powerMin.setPadding(new Insets(0, 5, 0, 0));
        powerMin.setBorder(new Border(new BorderStroke(Color.BLACK, BorderStrokeStyle.SOLID, CornerRadii.EMPTY, BorderStroke.THIN)));

        Label powerMaxLabel = new Label(LocaleManager.getTranslation("label.power.max.name"));
        powerMaxLabel.setPrefWidth(184);
        powerMax = new Label("0.000W");
        powerMax.setPrefSize(100, 32);
        powerMax.setAlignment(Pos.CENTER_RIGHT);
        powerMax.setPadding(new Insets(0, 5, 0, 0));
        powerMax.setBorder(new Border(new BorderStroke(Color.BLACK, BorderStrokeStyle.SOLID, CornerRadii.EMPTY, BorderStroke.THIN)));

        powerInfoPane.addColumn(0, powerNowLabel, powerMinLabel, powerMaxLabel);
        powerInfoPane.addColumn(1, powerNow, powerMin, powerMax);

        Button resetPower = new Button(LocaleManager.getTranslation("button.reset.minmax.name"));
        resetPower.setOnMouseClicked(event -> resetMinMaxPower());
        powerInfoPane.add(resetPower, 0, 5, 2, 1);

        powerPane.add(powerInfoPane, 1, 0);
    }

    private void initLiveDisplay(GridPane pane)
    {
        pane.setPadding(new Insets(30));
        pane.setAlignment(Pos.CENTER);

        voltageLive = new Label("0.00V ");
        voltageLive.setStyle("-fx-font-size: 48px;");
        voltageLive.setPrefWidth(300);
        voltageLive.setAlignment(Pos.CENTER_RIGHT);
        pane.add(voltageLive, 0, 0);

        currentLive = new Label("0.00mA");
        currentLive.setStyle("-fx-font-size: 48px;");
        currentLive.setPrefWidth(300);
        currentLive.setAlignment(Pos.CENTER_RIGHT);
        pane.add(currentLive, 0, 1);

        powerLive = new Label("0.000W ");
        powerLive.setStyle("-fx-font-size: 48px;");
        powerLive.setPrefWidth(300);
        powerLive.setAlignment(Pos.CENTER_RIGHT);
        pane.add(powerLive, 0, 2);
    }

    private void initLogInfoDisplay(GridPane pane)
    {
        pane.setPadding(new Insets(30));

        Label fileNameLabel = new Label(LocaleManager.getTranslation("label.logfile.name"));
        pane.add(fileNameLabel, 0, 0);
        fileName = new TextField();
        fileName.setEditable(false);
        pane.add(fileName, 1, 0);
        Button searchFile = new Button(LocaleManager.getTranslation("label.search.name"));
        searchFile.setOnMouseClicked(event -> openFileChooser());
        pane.add(searchFile, 2, 0);
        Label dataPointCountLabel = new Label(LocaleManager.getTranslation("label.datapoints.name"));
        pane.add(dataPointCountLabel, 0, 1);
        dataPointCount = new Label("0");
        dataPointCount.setPrefWidth(410);
        dataPointCount.setAlignment(Pos.CENTER_RIGHT);
        pane.add(dataPointCount, 1, 1);
        Label fileSizeLabel = new Label(LocaleManager.getTranslation("label.filesize.name"));
        pane.add(fileSizeLabel, 0, 2);
        fileSize = new Label("0 Bytes");
        fileSize.setPrefWidth(410);
        fileSize.setAlignment(Pos.CENTER_RIGHT);
        pane.add(fileSize, 1, 2);
        startLogging = new Button(LocaleManager.getTranslation("button.logger.start.name"));
        startLogging.setPrefWidth(412);
        startLogging.disableProperty().bind(fileName.textProperty().isEmpty());
        startLogging.setOnMouseClicked((event) -> Main.INSTANCE.getDataManager().startLogging());
        pane.add(startLogging, 1, 3);
    }

    private void initCommInfoDisplay(GridPane pane)
    {
        pane.setPadding(new Insets(30));
        //pane.setAlignment(Pos.CENTER);

        Label connectedLabel = new Label(LocaleManager.getTranslation("label.connected.name"));
        connected = new Label(LocaleManager.getTranslation("label.connected.false.name"));
        connected.setTextFill(Color.RED);
        pane.add(connectedLabel, 0, 0);
        pane.add(connected, 1, 0);

        Label portsLabel = new Label(LocaleManager.getTranslation("label.ports.name"));
        portsLabel.setVisible(!Main.INSTANCE.getConfigHandler().getUseHid());
        pane.add(portsLabel, 0, 1);
        portList = new ComboBox<>();
        portList.setPrefWidth(145);
        portList.setVisible(!Main.INSTANCE.getConfigHandler().getUseHid());
        pane.add(portList, 1, 1);

        scanPorts = new Button(LocaleManager.getTranslation("button.scan.name"));
        scanPorts.setPrefWidth(412);
        scanPorts.setOnMouseClicked(event -> Main.INSTANCE.getCommHandler().listAllPorts());
        scanPorts.setVisible(!Main.INSTANCE.getConfigHandler().getUseHid());
        pane.add(scanPorts, 1, 3);

        connect = new Button(LocaleManager.getTranslation("button.connect.name"));
        connect.setPrefWidth(412);
        connect.setDisable(true);
        connect.setOnMouseClicked(event ->
        {
            if (connStatus == CommStatus.CONNECTED)
            {
                Main.INSTANCE.getCommHandler().scheduleDisconnect();
            }
            else
            {
                Main.INSTANCE.getCommHandler().scheduleConnect(portList.getValue());
            }
        });
        pane.add(connect, 1, 4);
    }

    private void initMiscButtons(GridPane pane)
    {
        pane.setPadding(new Insets(50));
        pane.setPrefHeight(559);
        pane.setAlignment(Pos.TOP_CENTER);

        resetAllDisplays = new Button(LocaleManager.getTranslation("button.reset.all.name"));
        resetAllDisplays.setPrefWidth(412);
        resetAllDisplays.setOnMouseClicked(event -> resetAllDisplays());
        pane.add(resetAllDisplays, 0, 0);

        resetGraphs = new Button(LocaleManager.getTranslation("button.reset.graphs.name"));
        resetGraphs.setPrefWidth(412);
        resetGraphs.setOnMouseClicked(event -> resetGraphs());
        pane.add(resetGraphs, 0, 1);

        Button resetAllMinMax = new Button(LocaleManager.getTranslation("button.reset.minmax.name"));
        resetAllMinMax.setPrefWidth(412);
        resetAllMinMax.setOnMouseClicked(event -> resetAllMinMax());
        pane.add(resetAllMinMax, 0, 2);
    }

    //Public methods
    //TODO: implement, if possible, marking the over current events on the graph when they are enabled
    public void configureDisplay(int logInterval, int maxCurrent, boolean logOCEvents)
    {
        this.logInterval = logInterval;
        maxItems = (MAX_TIME_WIDTH / logInterval) + 2;

        if(currentLimit != maxCurrent)
        {
            for (XYChart.Data<Number, Number> value : seriesCurrent.getData())
            {
                if (value.getYValue().doubleValue() > (double) maxCurrent)
                {
                    currentLimitPostponed = true;
                    break;
                }
            }
            this.currentLimit = maxCurrent;
        }

        if(!currentLimitPostponed)
        {
            yAxisCurrent.setUpperBound(maxCurrent);
            yAxisCurrent.setTickUnit(Math.max(maxCurrent / 10, 1));
        }
        yAxisPower.setUpperBound(MAX_VOLTAGE * maxCurrent);
        yAxisPower.setTickUnit((MAX_VOLTAGE * maxCurrent) / 10);
    }

    public void resetGraphs()
    {
        //Reset X axis
        xAxisVoltage.setLowerBound(0);
        xAxisVoltage.setUpperBound(MAX_TIME_WIDTH);

        xAxisCurrent.setLowerBound(0);
        xAxisCurrent.setUpperBound(MAX_TIME_WIDTH);

        xAxisPower.setLowerBound(0);
        xAxisPower.setUpperBound(MAX_TIME_WIDTH);

        //Reset y axis
        yAxisVoltage.setLowerBound(0);
        yAxisVoltage.setUpperBound(MAX_VOLTAGE);

        yAxisCurrent.setLowerBound(0);
        yAxisCurrent.setUpperBound(MAX_CURRENT);
        yAxisCurrent.setTickUnit(MAX_CURRENT / 10D);

        yAxisPower.setLowerBound(0);
        yAxisPower.setUpperBound(MAX_VOLTAGE * MAX_CURRENT);
        yAxisPower.setTickUnit((MAX_VOLTAGE * MAX_CURRENT) / 10);

        //Clear values
        seriesVoltage.getData().clear();
        seriesCurrent.getData().clear();
        seriesPower.getData().clear();
    }

    public void addDataPoint(long timestamp, double voltage, double current, double power)
    {
        //If necessary, move the time bounds
        if(timestamp > MAX_TIME_WIDTH)
        {
            xAxisVoltage.setLowerBound(xAxisVoltage.getLowerBound() + (timestamp - lastTimestamp));
            xAxisVoltage.setUpperBound(timestamp);

            xAxisCurrent.setLowerBound(xAxisCurrent.getLowerBound() + (timestamp - lastTimestamp));
            xAxisCurrent.setUpperBound(timestamp);

            xAxisPower.setLowerBound(xAxisPower.getLowerBound() + (timestamp - lastTimestamp));
            xAxisPower.setUpperBound(timestamp);
        }
        lastTimestamp = timestamp;

        //Actually add the data points
        seriesVoltage.getData().add(new XYChart.Data<>(timestamp, voltage));
        seriesCurrent.getData().add(new XYChart.Data<>(timestamp, current));
        seriesPower.getData().add(new XYChart.Data<>(timestamp, power));

        //Remove obsolete data points
        if(seriesVoltage.getData().size() > maxItems) { seriesVoltage.getData().remove(0); }
        if(seriesCurrent.getData().size() > maxItems) { seriesCurrent.getData().remove(0); }
        if(seriesPower.getData().size() > maxItems) { seriesPower.getData().remove(0); }

        //Try to shrink the current graph y axis if the current limit was lowered while a larger value was displayed
        if(currentLimitPostponed)
        {
            boolean canShrinkGraph = true;
            for (XYChart.Data<Number, Number> value : seriesCurrent.getData())
            {
                if (value.getYValue().doubleValue() > currentLimit)
                {
                    canShrinkGraph = false;
                    break;
                }
            }

            if(canShrinkGraph)
            {
                yAxisCurrent.setUpperBound(maxCurrent);
                yAxisCurrent.setTickUnit(Math.max(maxCurrent / 10, 1));
                currentLimitPostponed = false;
            }
        }

        //Set text based displays
        voltageNow.setText(doubleToString(voltage, "V"));
        if(voltage < minVoltage) { voltageMin.setText(doubleToString(minVoltage = voltage, "V")); }
        if(voltage > maxVoltage) { voltageMax.setText(doubleToString(maxVoltage = voltage, "V")); }

        currentNow.setText(doubleToString(current, "mA"));
        if(current < minCurrent) { currentMin.setText(doubleToString(minCurrent = current, "mA")); }
        if(current > maxCurrent) { currentMax.setText(doubleToString(maxCurrent = current, "mA")); }

        powerNow.setText(doubleToString(power / 1000D, "W", 3));
        if(power < minPower) { powerMin.setText(doubleToString((minPower = power) / 1000D, "W", 3)); }
        if(power > maxPower) { powerMax.setText(doubleToString((maxPower = power) / 1000D, "W", 3)); }

        //Set big live display
        voltageLive.setText(doubleToString(voltage, "V "));
        currentLive.setText(doubleToString(current, "mA"));
        powerLive.setText(doubleToString(power / 1000D, "W ", 3));
    }

    public void signalDataStart()
    {
        resetAllDisplays.setDisable(true);
        resetGraphs.setDisable(true);
    }

    public void signalDataEnd()
    {
        resetAllDisplays.setDisable(false);
        resetGraphs.setDisable(false);
    }

    public void setLoggingStatus(boolean logging)
    {
        Platform.runLater(() ->
        {
            startLogging.setText(logging ? LocaleManager.getTranslation("button.logger.stop.name") : LocaleManager.getTranslation("button.logger.start.name"));

            if(logging)
            {
                startLogging.setOnMouseClicked((event) -> Main.INSTANCE.getDataManager().stopLogging());
            }
            else
            {
                startLogging.setOnMouseClicked((event) -> Main.INSTANCE.getDataManager().startLogging());
            }
        });
    }

    public void setConnectionStatus(CommStatus status)
    {
        connStatus = status;

        switch (status)
        {
            case DISCONNECTED:
            {
                connected.setText(LocaleManager.getTranslation("label.connected.false.name"));
                connected.setTextFill(Color.RED);
                Main.INSTANCE.getCommHandler().listAllPorts();
                connect.setDisable(portList.getItems().isEmpty());
                connect.setText(LocaleManager.getTranslation("button.connect.name"));
                scanPorts.setDisable(Main.INSTANCE.getConfigHandler().getUseHid());
                break;
            }
            case CONNECTING:
            {
                connected.setText(LocaleManager.getTranslation("label.connected.trying.name"));
                connected.setTextFill(Color.DARKORANGE);
                connect.setDisable(true);
                scanPorts.setDisable(true);
                break;
            }
            case WAITING_RESPONSE:
            {
                connected.setText(LocaleManager.getTranslation("label.connected.waiting.name"));
                break;
            }
            case CONNECTED:
            {
                connected.setText(LocaleManager.getTranslation("label.connected.true.name"));
                connected.setTextFill(Color.LIMEGREEN);
                connect.setDisable(false);
                connect.setText(LocaleManager.getTranslation("button.disconnect.name"));
                break;
            }
        }
    }

    public void updatePortList(String[] portArray)
    {
        portList.getItems().clear();
        List<String> ports = new  ArrayList<>(Arrays.asList(portArray));
        ports.remove("COM1");
        connect.setDisable(ports.isEmpty());
        portList.getItems().addAll(ports);
        portList.setValue(ports.isEmpty() ? null : ports.get(0));
    }

    public void updateHIDAvailable(boolean available) { connect.setDisable(!available); }

    public void updateLogInfo(int dataPoints, int byteCount)
    {
        dataPointCount.setText(Integer.toString(dataPoints));
        fileSize.setText(byteCount + " " + LocaleManager.getTranslation("label.bytes.name"));
    }

    public void shutdown()
    {

    }

    //Private handler methods
    private void resetAllDisplays()
    {
        resetGraphs();
        resetAllMinMax();
    }

    private void resetAllMinMax()
    {
        //Reset min/max values and clear texts
        resetMinMaxVoltage();
        resetMinMaxCurrent();
        resetMinMaxPower();
        voltageNow.setText("0.00V");
        currentNow.setText("0.00mA");
        powerNow.setText("0.000W");
        voltageLive.setText("0.00V ");
        currentLive.setText("0.00mA");
        powerLive.setText("0.000W ");
    }

    private void resetMinMaxVoltage()
    {
        minVoltage = 10;
        maxVoltage = 0;
        voltageMin.setText("0.00V");
        voltageMax.setText("0.00V");
    }

    private void resetMinMaxCurrent()
    {
        minCurrent = MAX_CURRENT;
        maxCurrent = 0;
        currentMin.setText("0.00mA");
        currentMax.setText("0.00mA");
    }

    private void resetMinMaxPower()
    {
        minPower = 20000;
        maxPower = 0;
        powerMin.setText("0.000W");
        powerMax.setText("0.000W");
    }

    private void openFileChooser()
    {
        FileChooser fileChooser = new FileChooser();
        fileChooser.getExtensionFilters().add(new FileChooser.ExtensionFilter(LocaleManager.getTranslation("desc.csv.name"), "*.csv"));
        File logFile = fileChooser.showSaveDialog(Main.INSTANCE.getOwnerWindow());

        if (logFile != null)
        {
            fileName.setText(logFile.getAbsolutePath());
            Main.INSTANCE.getDataManager().setLogFile(logFile);
        }
    }

    private void openHIDSettings()
    {
        Dialog<Pair<Boolean, String>> hidSettings = new Dialog<>();
        hidSettings.setTitle(LocaleManager.getTranslation("dialog.hid.name"));
        hidSettings.setHeaderText(LocaleManager.getTranslation("dialog.header.hid.1.name") + "\n" +
                LocaleManager.getTranslation("dialog.header.hid.2.name"));

        GridPane pane = createGridPane(10, 10);

        Label useHidLabel = new Label(LocaleManager.getTranslation("label.usehid.name"));
        pane.add(useHidLabel, 0, 0);

        CheckBox useHidButton = new CheckBox();
        useHidButton.setSelected(Main.INSTANCE.getConfigHandler().getUseHid());
        pane.add(useHidButton, 1, 0);

        Label serialNumLabel = new Label(LocaleManager.getTranslation("label.serialnum.name"));
        pane.add(serialNumLabel, 0, 1);

        TextField serialNumField = new TextField(Main.INSTANCE.getConfigHandler().getProductSerialNumber());
        serialNumField.setPrefColumnCount(20);
        pane.add(serialNumField, 1, 1);

        hidSettings.getDialogPane().setContent(pane);

        ButtonType okayButton = new ButtonType(LocaleManager.getTranslation("button.ok.name"), ButtonBar.ButtonData.APPLY);
        ButtonType cancelButton = new ButtonType(LocaleManager.getTranslation("button.cancel.name"), ButtonBar.ButtonData.CANCEL_CLOSE);
        hidSettings.getDialogPane().getButtonTypes().addAll(okayButton, cancelButton);

        hidSettings.setResultConverter(value ->
        {
            if (value.getButtonData() == ButtonBar.ButtonData.CANCEL_CLOSE) { return null; }

            return new Pair<>(useHidButton.isSelected(), serialNumField.getText());
        });

        Optional<Pair<Boolean, String>> result = hidSettings.showAndWait();
        if (result.isPresent())
        {
            Pair<Boolean, String> values = result.get();
            Main.INSTANCE.getConfigHandler().setUseHid(values.getKey());
            Main.INSTANCE.getConfigHandler().setProductSerialNumber(values.getValue());
        }
    }

    //Helpers
    private static GridPane createGridPane(int hGap, int vGap)
    {
        GridPane pane = new GridPane();
        pane.setHgap(hGap);
        pane.setVgap(vGap);
        return pane;
    }

    private static String doubleToString(double value, String unit) { return doubleToString(value, unit, 2); }

    private static String doubleToString(double value, String unit, int decimals)
    {
        String result = Double.toString(value);
        int diff = result.length() - result.indexOf('.');
        if(diff > decimals + 1) { result = result.substring(0, result.indexOf('.') + 3); }
        else if(diff < decimals + 1)
        {
            while (diff < decimals + 1)
            {
                //noinspection StringConcatenationInLoop
                result += '0';
                diff = result.length() - result.indexOf('.');
            }
        }
        return result + unit;
    }
}