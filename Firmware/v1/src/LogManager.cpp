#include <LogManager.hpp>

bool connected = false;
bool serialAvailable = false;
bool settingsChanged = false;
unsigned int logInterval = 100;
bool logOCEvents = false;
unsigned long logStartStamp = 0;
unsigned long lastLogStamp = 0;
unsigned long lastACKStamp = 0;

uint8_t dataIn[32];
uint8_t dataOut[32];

void initLogManager() {
  CommHandler::initComms();
  return; //TODO: remove when config (de)serialization is fixed
  //logInterval = getConfig()["logInterval"];
  //logOCEvents = getConfig()["logOCEvents"];
}

void updateLogger() {
  if(CommHandler::available()) {
    int charsRead = CommHandler::readBuffer(dataIn, 32);
    if(charsRead > 0) {
      switch (dataIn[0]) {
      case CMD_START: {
        connected = true;
        lastACKStamp = millis();
        CommHandler::writeByte(CMD_START_ACK);
        scheduleRedrawMain(REDRAW_STATE);
        delay(10);
        sendLoggerConfig();
        delay(10);
        break;
      }
      case CMD_STOP: {
        connected = false;
        logStartStamp = 0; //FIXME: sometimes doesn't reset properly
        scheduleRedrawMain(REDRAW_STATE);
        break;
      }
      case CMD_CFG_ACK:
      case CMD_DATA_ACK: {
        lastACKStamp = millis();
        break;
      }
      
      default: break;
      }
    }
  }

  if(!connected) { return; }

  if(settingsChanged) { sendLoggerConfig(); }

  if(millis() - lastACKStamp > (logInterval * 2)) { //Did we loose connection?
    connected = false;
    scheduleRedrawMain(REDRAW_STATE);
  }
}

bool getConnected() {
  return connected;
}

/**
 * Sends a data point to the computer
 * @param timestamp The actual timestamp of the reading, not the relative stamp for logging
 * @param voltage The measured voltage
 * @param current The measured current
 * @param power The measured power
*/
void sendDataPoint(unsigned long timestamp, float voltage, float current, float power) {
  if(!connected) { return; }
  if(timestamp - lastLogStamp < logInterval) { return; }

  if(logStartStamp == 0) { logStartStamp = timestamp; }

  lastLogStamp = timestamp;

  unsigned long relativeStamp = timestamp - logStartStamp;

  uint8_t buffer[17];
  buffer[0] = CMD_DATA_POINT;
  memcpy(buffer + 1, &relativeStamp, sizeof(unsigned long));
  memcpy(buffer + 5, &voltage, sizeof(float));
  memcpy(buffer + 9, &current, sizeof(float));
  memcpy(buffer + 13, &power, sizeof(float));
  CommHandler::writeBuffer(buffer, sizeof(buffer));
}

void updateLoggerConfig() {
  //getConfig()["logInterval"] = logInterval;
  //getConfig()["logOCEvents"] = logOCEvents;
}

void setInitialLoggerConfig() {
  //getConfig()["logInterval"] = 100;
  //getConfig()["logOCEvents"] = false;
}

void sendLoggerConfig() {
  settingsChanged = false;

  uint8_t buffer[10];
  buffer[0] = CMD_CFG;
  memcpy(buffer + 1, &logInterval, sizeof(unsigned int));
  memcpy(buffer + 5, getCurrentLimitPtr(), sizeof(int));
  memcpy(buffer + 9, &logOCEvents, sizeof(bool));
  CommHandler::writeBuffer(buffer, sizeof(buffer));
}

void setSettingsChanged(bool changed) {
  settingsChanged = changed;
}

unsigned int getLogInterval() { return logInterval; }
void setLogInterval(unsigned int interval) { logInterval = interval; }
unsigned int* getLogIntervalPtr() { return &logInterval; }

bool getLogOCEvents() { return logOCEvents; }
void setLogOCEvents(bool log) { logOCEvents = log; }
bool* getLogOCEventsPtr() { return &logOCEvents; }