#ifndef DISP_HANDLER_H
#define DISP_HANDLER_H

#include <Arduino.h>

#include <PowerController.hpp>
#include <Defines.hpp>
#include <PinMap.hpp>
#include "../lib/SSD1306_Menu/Menu.hpp"

#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

#define REDRAW_ALL 1
#define REDRAW_MEASURE 2
#define REDRAW_STATE 3

#define PIXEL_DUTY 500
#define PIXEL_OFF 0
#define PIXEL_ON 1
#define PIXEL_ERROR 2

void initDisplayHandler();
void updateDisplay();

void displayMainScreen();
void updateMainScreen();

void scheduleRedrawMain(int mode);

void openMenu();
bool isMenuOpen();

MainMenu* getMenu();

void clearLine(int x, int y, int width);

void setPixelOff();
void setPixelOn();
void setPixelError();

void updateDisplaySettings();
bool switchDisplayOff();
void switchDisplayOn();
bool isDisplayOff();

#endif