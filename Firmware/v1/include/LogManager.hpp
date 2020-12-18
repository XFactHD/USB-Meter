#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include <Arduino.h>

#include <ConfigHandler.hpp>
#include <MenuHandler.hpp>
#include <PowerController.hpp>
#include <CommHandler.hpp>

#define CMD_START 0xA0
#define CMD_START_ACK 0xA5
#define CMD_CFG 0xB0
#define CMD_CFG_ACK 0xB5
#define CMD_DATA_POINT 0xC0
#define CMD_DATA_ACK 0xC5
#define CMD_STOP 0xF0

void initLogManager();
void updateLogger();
bool getConnected();
bool startStopLogger();
void sendDataPoint(unsigned long timestamp, float voltage, float current, float power);

unsigned int getLogInterval();
void setLogInterval(unsigned int interval);
unsigned int* getLogIntervalPtr();

bool getLogOCEvents();
void setLogOCEvents(bool log);
bool* getLogOCEventsPtr();

void updateLoggerConfig();
void setInitialLoggerConfig();
void sendLoggerConfig();
void setSettingsChanged(bool changed);

#endif