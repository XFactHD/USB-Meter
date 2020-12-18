#ifndef USB2512_H
#define USB2512_H

#include <Arduino.h>
#include <Wire.h>

#define BYTE_COUNT_OFFSET(set) ((set) + 1)
#define DATA_OFFSET(set) ((set) + 2)

//I2C slave address
#define USB2512_SLAVE_ADDRESS 0x2C //0101100b

//Register address defines
#define REG_VENDOR_ID_LSB 0x00
#define REG_VENDOR_ID_MSB 0x01
#define REG_PRODUCT_ID_LSB 0x02
#define REG_PRODUCT_ID_MSB 0x03
#define REG_DEVICE_ID_LSB 0x04
#define REG_DEVICE_ID_MSB 0x05
#define REG_CFG_BYTE_1 0x06
#define REG_CFG_BYTE_2 0x07
#define REG_CFG_BYTE_3 0x08
#define REG_NON_REM_DEV 0x09
#define REG_MAX_POWER_SELF 0x0C      //Hub + embedded peripheral max current
#define REG_MAX_POWER_BUS 0x0D       //Hub + embedded peripheral max current
#define REG_HC_MAX_CURRENT_SELF 0x0E //Hub controller max current
#define REG_HC_MAX_CURRENT_BUS 0x0F  //Hub controller max current
#define REG_MANUF_STR_LEN 0x13
#define REG_PROD_STR_LEN 0x14
#define REG_SER_STR_LEN 0x15
#define REG_MANUF_STR 0x16
#define REG_PRODUCT_STR 0x54
#define REG_SERIAL_STR 0x92
#define REG_BOOST_UP 0xF6
#define REG_BOOST_DOWN 0xF8
#define REG_PORT_SWAP 0xFA
#define REG_STATUS_CMD 0xFF

//Settings array constants
#define SETTING_VENDOR_ID_LSB 0
#define SETTING_VENDOR_ID_MSB 1
#define SETTING_PRODUCT_ID_LSB 2
#define SETTING_PRODUCT_ID_MSB 3
#define SETTING_DEVICE_ID_LSB 4
#define SETTING_DEVICE_ID_MSB 5
#define SETTING_CFG_BYTE_1 6
#define SETTING_CFG_BYTE_2 7
#define SETTING_CFG_BYTE_3 8
#define SETTING_NON_REM_DEV 9
#define SETTING_MANUF_STR_LEN 10
#define SETTING_PROD_STR_LEN 11
#define SETTING_SER_STR_LEN 12
#define SETTING_BOOST_UP 13
#define SETTING_BOOST_DOWN 14
#define SETTING_PORT_SWAP 15

//Non removable ports
#define USB2512_NRP_NONE 0  // No embedded device
#define USB2512_NRP_FIRST 1 // Embedded device on port 1
#define USB2512_NRP_BOTH 2  // Embedded device on both ports

//Port power switching
#define USB2512_PSW_GANGED false    // One power switch for all ports
#define USB2512_PSW_INDIVIDUAL true // One power switch per port

//PHY Boost setting
#define USB2512_BOOST_NONE 0   // No PHY boost
#define USB2512_BOOST_LOW 1    // 4% PHY boost
#define USB2512_BOOST_MEDIUM 2 // 8% PHY boost
#define USB2512_BOOST_HIGH 3   // 12% PHY boost

class USB2512 {
public:
  USB2512(int nonRemP, bool selfPowered, bool pPowerSw);
  void setVendorID(uint16_t id);
  void setProductID(uint16_t id);
  void setDeviceID(uint16_t id);
  void setManufacturerString(const char* manuf);
  void setProductString(const char* product);
  void setSerialString(const char* serial);
  void setPortSwapped(bool us, bool p0, bool p1);
  void setPortBoost(uint8_t us, uint8_t p0, uint8_t p1);
  int init();
private:
  void sendConfigSetting(uint8_t setting, uint8_t byteCount, uint8_t value, uint8_t* error);
  void sendDescriptorString(uint8_t setting, uint8_t len, const char* ascii, uint8_t* error);

  bool initialized;
  const char* manufString;
  const char* productString;
  const char* serialString;

  uint8_t settings[16] = {
    0x24,       // Vendor ID LSB
    0x04,       // Vendor ID MSB
    0x12,       // Product ID LSB
    0x25,       // Product ID MSB
    0xB3,       // Device ID LSB
    0x0B,       // Device ID MSB
    B00110000,  // CFG Byte 1
    B00000000,  // CFG Byte 2
    B00000000,  // CFG Byte 3
    B00000000,  // Non Rem Dev
    0,          // Manufacturer String length
    0,          // Product String length
    0,          // Serial String length
    B00000000,  // Boost up
    B00000000,  // Boost down
    B00000000   // Port swap
  };
};

#endif