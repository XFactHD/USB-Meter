#include <PowerController.hpp>

Adafruit_INA219 meter = Adafruit_INA219();

unsigned int displayRefreshRate = 50;
unsigned int readsPerDispRefresh = displayRefreshRate / MEASURE_INTERVAL;

unsigned int spikeDebounce = 0;
unsigned int currentLimit = 500;

bool wasFault = false;
bool fault = false;
bool outputOn = false;

float voltage = 0;
float current = 0;
float power = 0;

unsigned long lastReadStamp = 0;
unsigned long ocStartStamp = 0;
unsigned int readsSinceDispRefresh = 0;

void initPowerController() {
  pinMode(SWITCH_FAULT, INPUT);
  pinMode(POWER_SWITCH, OUTPUT);

  //spikeDebounce = getConfig()["spikeDebounce"];
  //currentLimit = getConfig()["currentLimit"];

  meter.begin();
  //TODO: set calibration values for 8V and 3.2A
}

void checkSwitch() {
  fault = !digitalRead(SWITCH_FAULT);

  if(fault && !wasFault) {
    wasFault = fault;

    switchPower(false);
    setPixelError();
  }
}

void readData() {
  unsigned long ms = millis();

  if(ms - lastReadStamp > MEASURE_INTERVAL) {
    lastReadStamp = ms;

    voltage = meter.getBusVoltage_V();
    current = meter.getCurrent_mA();
    current = max(current, 0.0F); // Remove small negative jumps when switch is off or no load is attached
    power = meter.getPower_mW();

    readsSinceDispRefresh++;
    if(readsSinceDispRefresh > readsPerDispRefresh) {
      readsSinceDispRefresh = 0;
      scheduleRedrawMain(REDRAW_MEASURE);
    }

    checkMeasurements(ms, voltage, current);

    sendDataPoint(ms, voltage, current, power);
  }
}

void checkMeasurements(unsigned long stamp, float voltage, float current) {
  if(current > currentLimit) {
    if(ocStartStamp == 0) { ocStartStamp = stamp; }

    if(stamp - ocStartStamp >= spikeDebounce) {
      ocStartStamp = 0;

      switchPower(false);
      wasFault = true; //Only set wasFault because the fault clears "itself" by cutting power
      setPixelError();
    }
  }
}

void getLatestMeasurements(float* u, float* i, float* p) {
  *u = voltage;
  *i = current;
  *p = power;
}

void switchPower() {
  if(wasFault) {
    if(!fault) { //If the fault is cleared
      wasFault = false;
      setPixelOff();
    }
  }
  else {
    switchPower(!outputOn);
  }
}

void switchPower(bool state) {
  outputOn = state;
  digitalWrite(POWER_SWITCH, state);
  scheduleRedrawMain(REDRAW_STATE);

  if(state) { setPixelOn(); }
  else { setPixelOff(); }
}

bool getOutputState() {
  return outputOn;
}

unsigned int getSpikeDebounce() { return spikeDebounce; }
void setSpikeDebounce(unsigned int debounce) { spikeDebounce = debounce; }
unsigned int* getSpikeDebouncePtr() { return &spikeDebounce; }

unsigned int getCurrentLimit() { return currentLimit; }
void setCurrentLimit(unsigned int limit) { currentLimit = limit; }
unsigned int* getCurrentLimitPtr() { return &currentLimit; }
bool setCurrentLimitToMax() {
  currentLimit = CURRENT_LIMIT_MAX;
  return true;
}

unsigned int* getDisplayRefreshPtr() { return &displayRefreshRate; }
void updateRefreshRate() { readsPerDispRefresh = displayRefreshRate / MEASURE_INTERVAL; }

void updatePowerConfig() {
  setSettingsChanged(true);
  return; //TODO: remove when testing on final hardware
  //getConfig()["spikeDebounce"] = spikeDebounce;
  //getConfig()["currentLimit"] = currentLimit;
}

void setInitialPowerConfig() {
  return; //TODO: remove when testing on final hardware
  //getConfig()["spikeDebounce"] = 0;
  //getConfig()["currentLimit"] = 500;
}
