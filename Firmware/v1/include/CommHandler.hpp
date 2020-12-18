#ifndef COMM_HANDLER_H
#define COMM_HANDLER_H

#include <Arduino.h>

#define USE_HID

#ifdef USE_HID
  #include <Adafruit_TinyUSB.h>
#endif

namespace CommHandler {
  void initComms();

  void writeByte(uint8_t data);
  void writeBuffer(uint8_t* buffer, size_t size);

  int available();
  int readBuffer(uint8_t* buffer, size_t size);
}

#endif