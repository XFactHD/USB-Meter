#include <ButtonHandler.hpp>

bool stateUp = false;
bool stateDown = false;
bool stateLeft = false;
bool stateRight = false;
bool stateCenter = false;

unsigned long lastPressUp = 0;
unsigned long lastPressDown = 0;
unsigned long lastPressLeft = 0;
unsigned long lastPressRight = 0;
unsigned long lastPressCenter = 0;

unsigned long lastRepeatUp = 0;
unsigned long lastRepeatDown = 0;
unsigned long lastRepeatLeft = 0;
unsigned long lastRepeatRight = 0;

void initButtonHandler() {
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_CENTER, INPUT_PULLUP);
}

void checkButtons() {
  unsigned long ms = millis();

  bool btnUp = digitalRead(BUTTON_UP);
  bool btnDown = digitalRead(BUTTON_DOWN);
  bool btnLeft = digitalRead(BUTTON_LEFT);
  bool btnRight = digitalRead(BUTTON_RIGHT);
  bool btnCenter = digitalRead(BUTTON_CENTER);

  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;
  bool center = false;

  //Button up
  if(btnUp && !stateUp) {
    if(ms - lastPressUp > BUTTON_DEBOUNCE) {
      lastPressUp = ms;
      up = true;
    }
    stateUp = true;
  }
  /*else if(stateUp && btnUp && ms - lastRepeatUp > BUTTON_REPEAT_DELAY) {
    lastRepeatUp = ms;
    up = true;
  }*/
  else if(stateUp && !btnUp) {
    stateUp = false;
  }

  //Button down
  if(btnDown && !stateDown) {
    if(ms - lastPressDown > BUTTON_DEBOUNCE) {
      lastPressDown = ms;
      down = true;
    }
    stateDown = true;
  }
  /*else if(stateDown && btnDown && ms - lastRepeatDown > BUTTON_REPEAT_DELAY) {
    lastRepeatDown = ms;
    down = true;
  }*/
  else if(stateDown && !btnDown) {
    stateDown = false;
  }

  //Button left
  if(btnLeft && !stateLeft) {
    if(ms - lastPressLeft > BUTTON_DEBOUNCE) {
      lastPressLeft = ms;
      left = true;
    }
    stateLeft = true;
  }
  else if(stateLeft && btnLeft && ms - lastPressLeft > BUTTON_REPEAT_INIT && ms - lastRepeatLeft > BUTTON_REPEAT_DELAY) {
    lastRepeatLeft = ms;
    left = true;
  }
  else if(stateLeft && !btnLeft) {
    stateLeft = false;
  }

  //Button right
  if(btnRight && !stateRight) {
    if(ms - lastPressRight > BUTTON_DEBOUNCE) {
      lastPressRight = ms;
      right = true;
    }
    stateRight = true;
  }
  else if(stateRight && btnRight && ms - lastPressRight > BUTTON_REPEAT_INIT && ms - lastRepeatRight > BUTTON_REPEAT_DELAY) {
    lastRepeatRight = ms;
    right = true;
  }
  else if(stateRight && !btnRight) {
    stateRight = false;
  }

  //Button center (Center / OK doesn't need key repeat)
  if(btnCenter && !stateCenter) {
    if(ms - lastPressCenter > BUTTON_DEBOUNCE) {
      lastPressCenter = ms;
      center = true;
    }
    stateCenter = true;
  }
  else if(stateCenter && !btnCenter) {
    stateCenter = false;
  }

  if(isMenuOpen()) {
    getMenu()->handleButtons(up, down, left, right, center, stateUp, stateDown, stateLeft,  stateRight);
  }
  else {
    if(isDisplayOff() && (center || left || right || up || down)) { switchDisplayOn(); }
    else if(center) { getMenu()->open(); }
    else if(right) { switchPower(); }
  }
}
