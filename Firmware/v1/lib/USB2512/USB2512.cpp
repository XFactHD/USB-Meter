#include "USB2512.h"

USB2512::USB2512(int nonRemP, bool selfPowered, bool pPowerSw) {
  initialized = false;
  settings[SETTING_CFG_BYTE_1] |= (selfPowered << 7) | (selfPowered ? (pPowerSw << 1) : B100) | pPowerSw;
  settings[SETTING_CFG_BYTE_2] |= ((nonRemP != USB2512_NRP_NONE) << 3);
  settings[SETTING_NON_REM_DEV] |= (nonRemP << 1);
}

void USB2512::setVendorID(uint16_t id){
  if(initialized) { return; }
  settings[SETTING_VENDOR_ID_LSB] = id & 0xFF;
  settings[SETTING_VENDOR_ID_MSB] = (id >> 8) & 0xFF;
}

void USB2512::setProductID(uint16_t id){
  if(initialized) { return; }
  settings[SETTING_PRODUCT_ID_LSB] = id & 0xFF;
  settings[SETTING_PRODUCT_ID_MSB] = (id >> 8) & 0xFF;
}

void USB2512::setDeviceID(uint16_t id){
  if(initialized) { return; }
  settings[SETTING_DEVICE_ID_LSB] = id & 0xFF;
  settings[SETTING_DEVICE_ID_MSB] = (id >> 8) & 0xFF;
}

void USB2512::setManufacturerString(const char* manuf) {
  if(initialized || strlen(manuf) > 31) { return; }
  settings[SETTING_CFG_BYTE_3] |= 1;
  settings[SETTING_MANUF_STR_LEN] = strlen(manuf);
  manufString = manuf;
}

void USB2512::setProductString(const char* product) {
  if(initialized || strlen(product) > 31) { return; }
  settings[SETTING_CFG_BYTE_3] |= 1;
  settings[SETTING_PROD_STR_LEN] = strlen(product);
  productString = product;
}

void USB2512::setSerialString(const char* serial) {
  if(initialized || strlen(serial) > 31) { return; }
  settings[SETTING_CFG_BYTE_3] |= 1;
  settings[SETTING_SER_STR_LEN] = strlen(serial);
  serialString = serial;
}

void USB2512::setPortSwapped(bool us, bool ds0, bool ds1) {
  if(initialized) { return; }
  settings[SETTING_PORT_SWAP] |= (us | (ds0 << 1) | (ds1 << 2));

}

void USB2512::setPortBoost(uint8_t us, uint8_t p0, uint8_t p1) {
  if(initialized || us > 3 || p0 > 3 || p1 > 3) { return; }
  settings[SETTING_BOOST_UP] |= us;
  settings[SETTING_BOOST_DOWN] |= (p1 << 2) | p0;
}

int USB2512::init() {
  Wire.begin();
  uint8_t error = 0;
  
  sendConfigSetting(REG_VENDOR_ID_LSB,  1, settings[ 0], &error);
  sendConfigSetting(REG_VENDOR_ID_MSB,  1, settings[ 1], &error);
  sendConfigSetting(REG_PRODUCT_ID_LSB, 1, settings[ 2], &error);
  sendConfigSetting(REG_PRODUCT_ID_MSB, 1, settings[ 3], &error);
  sendConfigSetting(REG_DEVICE_ID_LSB,  1, settings[ 4], &error);
  sendConfigSetting(REG_DEVICE_ID_MSB,  1, settings[ 5], &error);

  sendConfigSetting(REG_CFG_BYTE_1,     1, settings[ 6], &error);
  sendConfigSetting(REG_CFG_BYTE_2,     1, settings[ 7], &error);
  sendConfigSetting(REG_CFG_BYTE_3,     1, settings[ 8], &error);

  sendConfigSetting(REG_NON_REM_DEV,    1, settings[ 9], &error);

  if(settings[SETTING_CFG_BYTE_1] & 0x80) { // Check for selfpowered mode
    sendConfigSetting(REG_MAX_POWER_SELF,   1, 50, &error);    // 100mA for complete compound device
    sendConfigSetting(REG_HC_MAX_CURRENT_SELF, 1, 25, &error); // 50mA for hub controller
  }
  else {
    sendConfigSetting(REG_MAX_POWER_BUS,    1, 50, &error);    // 100mA for complete compound device
    sendConfigSetting(REG_HC_MAX_CURRENT_BUS,  1, 25, &error); // 50mA for hub controller
  }

  if(settings[SETTING_CFG_BYTE_3] & 0x1) { // Check for string descriptor support
    sendConfigSetting(REG_MANUF_STR_LEN,  1, settings[10], &error);
    sendConfigSetting(REG_PROD_STR_LEN,   1, settings[11], &error);
    sendConfigSetting(REG_SER_STR_LEN,    1, settings[12], &error);

    sendDescriptorString(REG_MANUF_STR,   settings[SETTING_MANUF_STR_LEN] * 2, manufString,   &error);
    sendDescriptorString(REG_PRODUCT_STR, settings[SETTING_PROD_STR_LEN] * 2,  productString, &error);
    sendDescriptorString(REG_SERIAL_STR,  settings[SETTING_SER_STR_LEN] * 2,   serialString,  &error);
  }

  sendConfigSetting(REG_BOOST_UP,       1, settings[13], &error);
  sendConfigSetting(REG_BOOST_DOWN,     1, settings[14], &error);

  sendConfigSetting(REG_PORT_SWAP,      1, settings[15], &error);

  sendConfigSetting(REG_STATUS_CMD,     1,         0x01, &error);

  if(error != 0) { return error; }
  initialized = true;
  return 0;
}

void USB2512::sendDescriptorString(uint8_t setting, uint8_t len, const char* ascii, uint8_t* error) {
  Wire.beginTransmission(USB2512_SLAVE_ADDRESS);
  Wire.write(setting);
  Wire.write(len * 2);

  for (int i = 0; i < len; i++) {
    uint16_t utfChar = ascii[i];
    Wire.write(utfChar);
  }
  int err = Wire.endTransmission();
  if(err != 0) { *error = err; }
}

void USB2512::sendConfigSetting(uint8_t setting, uint8_t byteCount, uint8_t value, uint8_t* error) {
  Wire.beginTransmission(USB2512_SLAVE_ADDRESS);
  Wire.write(setting);
  Wire.write(byteCount);
  Wire.write(value);
  int err = Wire.endTransmission();
  if(err != 0) { *error = err; }
}
