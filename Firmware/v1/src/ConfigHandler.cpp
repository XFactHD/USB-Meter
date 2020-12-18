#include <ConfigHandler.hpp>

const int ADDRESS_ID = 0;     //Address of the version ID
const int ADDRESS_LENGTH = 2; //Address of the length of the data section
const int ADDRESS_DATA = 6;   //Address of the data section

const uint16_t id = 0x0000;

Adafruit_FRAM_SPI fram = Adafruit_FRAM_SPI(FRAM_CS);

void initConfigHandler() {
  fram.begin();
  readConfigFromFRAM();
}

void readConfigFromFRAM() {
  uint16_t idRead = 0;
  fram.read(ADDRESS_ID, (uint8_t*)&idRead, 2);
  if(idRead != id) {
    fram.writeEnable(true);
    fram.write(ADDRESS_ID, (uint8_t*)(&id), 2);
    fram.writeEnable(false);

    initializeConfig();
  }

}

void writeConfigToFRAM() {
  uint32_t length = 0;
  fram.writeEnable(true);
  fram.write(ADDRESS_LENGTH, (uint8_t*)(&length), sizeof(length));
  fram.writeEnable(false);

  fram.writeEnable(true);
  fram.write(ADDRESS_DATA, (uint8_t*)nullptr, length);
  fram.writeEnable(false);
}

/**
 * Initializes the document on first startup or version change
*/
void initializeConfig() {
  

  writeConfigToFRAM();
}

/*DynamicJsonDocument& getConfig() {
  //return config;
}*/