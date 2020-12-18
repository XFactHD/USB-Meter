#include <main.hpp>

USB2512 hub = USB2512(USB2512_NRP_FIRST, true, USB2512_PSW_INDIVIDUAL);

void setup() {
  PORT->Group[PORTB].PINCFG[15].bit.PMUXEN = 0; //Fix for error on PCB: QSPI (SERCOM4) is on the pin that is incorrectly used as SCL on the PCB
  pinMode(FRAM_CS, OUTPUT);
  pinMode(DISP_CS, OUTPUT);
  digitalWrite(FRAM_CS, HIGH);
  digitalWrite(DISP_CS, HIGH);

  initDisplayHandler();
  hub.setPortSwapped(false, true, false);
  hub.init();
  initConfigHandler();
  initPowerController();
  initMenuHandler();
  initButtonHandler();
  initLogManager();
  displayMainScreen();
  scheduleRedrawMain(REDRAW_ALL);
  updateMainScreen();
}

void loop() {
  checkSwitch();
  readData();
  checkButtons();
  updateLogger();
  updateDisplay();
}