#include <CommHandler.hpp>

#ifdef USE_HID
  #define BUFFER_SIZE 32

  const uint8_t desc_hid_report[] = {
    TUD_HID_REPORT_DESC_GENERIC_INOUT(32)
  };

  Adafruit_USBD_HID usb_hid;

  uint8_t inStart = 0;
  uint8_t inPointer = 0;
  uint8_t inLength = 0;
  uint8_t bufferIn[BUFFER_SIZE];

  uint8_t bufferOut[BUFFER_SIZE];

  uint16_t get_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqLen) { return 0; }

  void set_report_callback(uint8_t report_id, hid_report_type_t report_type, const uint8_t* buffer, uint16_t bufLen) {
    for(uint16_t i = 0; i < bufLen; i++) {
      bufferIn[inPointer] = buffer[i];
      inPointer = (inPointer + 1) % BUFFER_SIZE;
      inLength++;
    }
  }
#endif

void CommHandler::initComms() {
  USBDevice.detach();
  USBDevice.setID(0xF055, 0x0000);
  USBDevice.setManufacturerDescriptor("DC");
  USBDevice.setProductDescriptor("USB-Meter");
  USBDevice.attach();

  #ifdef USE_HID
    //TODO: disable usb cdc

    usb_hid.enableOutEndpoint(true);
    usb_hid.setPollInterval(1);
    usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
    usb_hid.setReportCallback(get_report_callback, set_report_callback);

    usb_hid.begin();
  #else
    Serial.begin(0);
  #endif
}

void CommHandler::writeByte(uint8_t data) {
  #ifdef USE_HID
    if(!USBDevice.mounted()) { return; }

    memset(bufferOut, 0, BUFFER_SIZE);
    bufferOut[0] = data;
    usb_hid.sendReport(0, bufferOut, 32);
  #else
    Serial.write(data);
  #endif
}

void CommHandler::writeBuffer(uint8_t* buffer, size_t size) {
  #ifdef USE_HID
    if(!USBDevice.mounted()) { return; }

    memset(bufferOut, 0, BUFFER_SIZE);
    size_t actualSize = min(size, BUFFER_SIZE);
    memcpy(bufferOut, buffer, actualSize);
    usb_hid.sendReport(0, bufferOut, BUFFER_SIZE);
  #else
    Serial.write(buffer, size);
  #endif
}

int CommHandler::available() {
  #ifdef USE_HID
    if(!USBDevice.mounted()/* || inStart == inPointer*/) { return 0; }

    return inLength;//(BUFFER_SIZE - max(inPointer, inStart) - 1) + min(inPointer, inStart);
  #else
    return Serial.available();
  #endif
}

int CommHandler::readBuffer(uint8_t* buffer, size_t size) {
  #ifdef USE_HID
    if(!USBDevice.mounted() || CommHandler::available() == 0) { return 0; }
    size_t count;
    for(count = 0; count < size; count++) {
      buffer[count] = bufferIn[inStart];

      inStart = (inStart + 1) % BUFFER_SIZE;
      inLength--;
      if(inStart == inPointer) { break; }
    }
    return count;
  #else
    return Serial.readBytes((char*)buffer, size);
  #endif
}