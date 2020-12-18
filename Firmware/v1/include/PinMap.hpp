#ifndef PIN_MAP_H
#define PIN_MAP_H

#define SWITCH_FAULT 11    //Low => power switch detected a fault (oc or temp) and throttled power output
#define POWER_SWITCH 10    //Low => power off, High => power on
#define EXT_POWER_DETECT 0 //High => external power is applied

#define DISP_DC 9
#define DISP_RESET 6
#define DISP_CS 5

#define FRAM_CS 12

#define BUTTON_UP A2
#define BUTTON_DOWN A5
#define BUTTON_LEFT A4
#define BUTTON_RIGHT A3
#define BUTTON_CENTER A1

#endif