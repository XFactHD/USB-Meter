#ifndef STORAGE_HANDLER_H
#define STORAGE_HANDLER_H

#include <Arduino.h>

#include <PowerController.hpp>
#include <LogManager.hpp>

#include <Adafruit_FRAM_SPI.h>

void initConfigHandler();
void readConfigFromFRAM();
void writeConfigToFRAM();
void initializeConfig();
//DynamicJsonDocument& getConfig();

#endif