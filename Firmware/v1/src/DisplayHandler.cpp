#include <DisplayHandler.hpp>

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &SPI, DISP_DC, DISP_RESET, DISP_CS);
Adafruit_NeoPixel pixel = Adafruit_NeoPixel(1, PIN_NEOPIXEL);

bool displayOff = false;
int redrawMain = 0;
int pixelState = PIXEL_OFF;
bool pixelErrorOn = false;
unsigned long lastPixelUpdate = 0;

MainMenu mainMenu = MainMenu("Main menu");

//Main functions
void initDisplayHandler() {
  display.begin();
  display.setTextColor(SSD1306_WHITE);
  display.display();

  pixel.begin();
  pixel.clear();
  pixel.show();
}

void updateDisplay() {
  if(pixelState == PIXEL_ERROR) {
    if(millis() - lastPixelUpdate > PIXEL_DUTY) {
      lastPixelUpdate = millis();
      pixelErrorOn = !pixelErrorOn;

      if(pixelErrorOn) { pixel.setPixelColor(0, Adafruit_NeoPixel::Color(1, 0, 0)); }
      else { pixel.clear(); }
      pixel.show();
    }
  }

  if(displayOff) { return; }

  if (!mainMenu.draw(&display)) {
    if(mainMenu.needsRedraw()) { //If the menu is closed and marked for redraw, then it was closed and needs to be replaced by the main screen
      redrawMain = REDRAW_ALL;

      displayMainScreen();
    }

    if(redrawMain) { updateMainScreen(); }
  }
}

void displayMainScreen() {
  if(displayOff) { return; }

  display.clearDisplay();

  display.setCursor(0, 0);
  display.print("Voltage:");
  display.setCursor(0, 8);
  display.print("Current:");
  display.setCursor(0, 16);
  display.print("Power:");
  display.setCursor(0, 24);
  display.print("I Limit:");

  display.setCursor(0, 48);
  display.print("Output:");
  display.setCursor(0, 56);
  display.print("Connected: ");

  display.display();
}

void updateMainScreen() {
  if(displayOff) { return; }

  if(redrawMain == REDRAW_MEASURE || redrawMain == REDRAW_ALL) {
    float voltage = 0;
    float current = 0;
    float power = 0;

    getLatestMeasurements(&voltage, &current, &power);

    clearLine(54,  0, 11);
    clearLine(54,  8, 11);
    clearLine(54, 16, 11);

    int space;

    display.setCursor(78, 0);
    display.print(voltage);
    display.print("V");

    space = 6 * (3 - (int)log10(current));
    space = constrain(space, 0, 6 * 3);
    display.setCursor(60 + space, 8);
    display.print(current);
    display.print("mA");

    space = 6 * (4 - (int)log10(power));
    display.setCursor(54 + space, 16);
    display.print(power);
    display.print("mW");

    unsigned int iLim = getCurrentLimit();
    space = 6 * (3 - (int)log10(iLim));
    display.setCursor(60 + space, 24);
    display.print(iLim);
    display.print(".00mA");
  }

  if(redrawMain == REDRAW_STATE || redrawMain == REDRAW_ALL) {
    clearLine(72, 48, 3);
    clearLine(72, 56, 5);

    display.setCursor(72, 48);
    display.print(getOutputState() ? "On" : "Off");
    display.setCursor(72, 56);
    display.print(getConnected() ? "True" : "False");
  }

  display.display();
  redrawMain = 0;
}

void scheduleRedrawMain(int mode) {
  if(redrawMain != 0 && redrawMain != mode) {
    redrawMain = REDRAW_ALL;
  }
  else {
    redrawMain = mode;
  }
}

void clearLine(int x, int y, int width) {
  display.fillRect(x, y, width * 6, 8, SSD1306_BLACK);
}

bool isMenuOpen() { return mainMenu.isOpen(); }

MainMenu* getMenu() { return &mainMenu; }

void setPixelOff() {
  pixelState = PIXEL_OFF;
  
  pixel.clear();
  pixel.show();
}

void setPixelOn() {
  pixelState = PIXEL_ON;

  pixel.setPixelColor(0, Adafruit_NeoPixel::Color(0, 1, 0));
  pixel.show();
}

void setPixelError() {
  pixelState = PIXEL_ERROR;

  pixel.setPixelColor(0, Adafruit_NeoPixel::Color(1, 0, 0));
  pixel.show();

  lastPixelUpdate = millis();
  pixelErrorOn = true;
}

void updateDisplaySettings() {
  updateRefreshRate();
}

bool switchDisplayOff() { //Returns a bool to satisfy MenuButton
  displayOff = true;
  
  display.clearDisplay();
  display.display();

  mainMenu.close();
  mainMenu.resetSelected();
  mainMenu.needsRedraw(); //Clear dirty flag

  return false;
}

void switchDisplayOn() {
  displayOff = false;

  redrawMain = REDRAW_ALL;
  displayMainScreen();
}

bool isDisplayOff() {
  return displayOff;
}
