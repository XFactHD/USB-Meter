#ifndef DEFINES_H
#define DEFINES_H

typedef enum {  //Enumeration of possible power states
  SWITCH_ERROR, //Switch fault active (overcurrent or overtemperature)
  OVERCURRENT,  //Measured current is above the configured limit
  OFF,          //No errors, power switch is off
  ON            //No errors, power switch is on
} power_state;

#define MEASURE_INTERVAL 10

//Display settings limits
#define DISPLAY_INTERVAL_MIN 20
#define DISPLAY_INTERVAL_MAX 500
#define DISPLAY_INTERVAL_INCREMENT MEASURE_INTERVAL

//Log interval limits
#define LOG_INTERVAL_MIN 20
#define LOG_INTERVAL_MAX 1000
#define LOG_INTERVAL_INCREMENT 10

//Current spike debounce limits (ms)
#define SPIKE_DEBOUNCE_MIN 0
#define SPIKE_DEBOUNCE_MAX 20

//Current limit (mA)
#define CURRENT_LIMIT_MIN 10 //Going lower than 10mA is useless because of jitter
#define CURRENT_LIMIT_MAX 2500

//Button debounce/repeat
#define BUTTON_DEBOUNCE 70
#define BUTTON_REPEAT_INIT 500
#define BUTTON_REPEAT_DELAY 100

//Daughter board config EEPROM
#define EEPROM_I2C_ADDRESS 0x00
#define EEPROM_ADDRESS_MAX_CURRENT 0x00 //Max current capability of the daughter board
#define EEPROM_ADDRESS_MAX_VOLTAGE 0x00 //Max voltage capability of the daughter board
#define EEPROM_ADDRESS_U_VARIABLE  0x00 //If the voltage is variable or not (false for USB, true for others)

#endif