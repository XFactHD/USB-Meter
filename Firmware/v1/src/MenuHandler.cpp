#include <MenuHandler.hpp>

Menu* mainMenuPtr;
BackItem backMain = BackItem(writeConfigToFRAM);

/** Overcurrent protection options **/
Menu menuOcpOptions = Menu("OCP");

UnsignedIntValue valueCurrentLimit = UnsignedIntValue(getCurrentLimitPtr(), CURRENT_LIMIT_MIN, CURRENT_LIMIT_MAX, 1, true);
MenuItem itemCurrentLimit = MenuItem("Max current", "mA", &valueCurrentLimit, true);

MenuButton buttonMaxCurrent = MenuButton("Set to max", "Set to max", setCurrentLimitToMax); //Use std/alt name system to force redraw

UnsignedIntValue valueSpikeDebounce = UnsignedIntValue(getSpikeDebouncePtr(), SPIKE_DEBOUNCE_MIN, SPIKE_DEBOUNCE_MAX);
MenuItem itemSpikeDebounce = MenuItem("Spike debounce", "ms", &valueSpikeDebounce, true);

BackItem backOCP = BackItem(updatePowerConfig);
/** Overcurrent protection options end **/

/** Options menu **/
Menu menuOptions = Menu("Options");

//Display options
Menu menuDisplayOptions = Menu("Display");

UnsignedIntValue valueDisplayRefresh = UnsignedIntValue(getDisplayRefreshPtr(), DISPLAY_INTERVAL_MIN, DISPLAY_INTERVAL_MAX, DISPLAY_INTERVAL_INCREMENT);
MenuItem itemDisplayRefresh = MenuItem("Refresh rate", "ms", &valueDisplayRefresh, true);

BackItem backDisplayOptions = BackItem(updateDisplaySettings);
//Display options end

//Logger options
Menu menuLoggerOptions = Menu("Logger");

UnsignedIntValue valueLogInterval = UnsignedIntValue(getLogIntervalPtr(), LOG_INTERVAL_MIN, LOG_INTERVAL_MAX, LOG_INTERVAL_INCREMENT);
MenuItem itemLogInterval = MenuItem("Log interval", "ms", &valueLogInterval, true);

BoolValue valueLogOCEvents = BoolValue(getLogOCEventsPtr());
MenuItem itemLogOCEvents = MenuItem("Log OC Events", "", &valueLogOCEvents, true);

BackItem backLogOptions = BackItem(updateLoggerConfig);
//Logger options end

BackItem backOptions = BackItem();
/** Options menu end **/

/** Display off button **/
MenuButton buttonDisplayOff = MenuButton("Display off", "", switchDisplayOff);
/** Display off button end **/

/** Info menu **/
Menu menuInfo = Menu("Info");

StringValue valueAuthor = StringValue("DC");
MenuItem itemAuthor = MenuItem("Author", "", &valueAuthor, false);

StringValue valueVersion = StringValue("0.0.1");
MenuItem itemVersion = MenuItem("Version", "", &valueVersion, false);

BackItem backInfo = BackItem();
/** Info menu end **/

void initMenuHandler() {
  mainMenuPtr = getMenu();

  //Main menu
  mainMenuPtr->addItem(&menuOcpOptions);
  mainMenuPtr->addItem(&menuOptions);
  mainMenuPtr->addItem(&buttonDisplayOff);
  mainMenuPtr->addItem(&menuInfo);
  mainMenuPtr->addItem(&backMain);

  menuOcpOptions.addItem(&itemCurrentLimit);
  menuOcpOptions.addItem(&buttonMaxCurrent);
  menuOcpOptions.addItem(&itemSpikeDebounce);
  menuOcpOptions.addItem(&backOCP);

  //Options menu
  menuOptions.addItem(&menuDisplayOptions);
  menuDisplayOptions.addItem(&itemDisplayRefresh);
  menuDisplayOptions.addItem(&backDisplayOptions);
  
  menuOptions.addItem(&menuLoggerOptions);
  menuLoggerOptions.addItem(&itemLogInterval);
  menuLoggerOptions.addItem(&itemLogOCEvents);
  menuLoggerOptions.addItem(&backLogOptions);
  
  menuOptions.addItem(&backOptions);

  //Info menu
  menuInfo.addItem(&itemAuthor);
  menuInfo.addItem(&itemVersion);
  menuInfo.addItem(&backInfo);
}