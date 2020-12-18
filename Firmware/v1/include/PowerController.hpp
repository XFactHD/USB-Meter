#ifndef PWR_CTRL_H
#define PWR_CTRL_H

#include <Arduino.h>

#include <DisplayHandler.hpp>
#include <ConfigHandler.hpp>
#include <PinMap.hpp>

#include <Adafruit_INA219.h>

void initPowerController();
void checkSwitch();
void readData();
void checkUSBStates();
void switchPower();
void switchPower(bool state);
bool getOutputState();

void checkMeasurements(unsigned long stamp, float voltage, float current);
void getLatestMeasurements(float* voltage, float* current, float* power);

unsigned int getSpikeDebounce();
void setSpikeDebounce(unsigned int debounce);
unsigned int* getSpikeDebouncePtr();

unsigned int getCurrentLimit();
void setCurrentLimit(unsigned int ma);
unsigned int* getCurrentLimitPtr();
bool setCurrentLimitToMax();

unsigned int* getDisplayRefreshPtr();
void updateRefreshRate();

void updatePowerConfig();
void setInitialPowerConfig();

void onSwitchError(); //ISR for switch fault output

#endif