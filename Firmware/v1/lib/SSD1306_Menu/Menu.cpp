#include <Menu.hpp>

//Menu implementation
void Menu::draw(Adafruit_SSD1306* gfx, int x, int y) {
  if(selectedMenu != nullptr && selectedMenu->isMenu()) {
    selectedMenu->draw(gfx, x, y);
    return;
  }

  gfx->setTextColor(COLOR_WHITE);
  gfx->setTextSize(1);

  int textLen = (int)strlen(name) * 12;
  int textX = 64 - (textLen / 2);
  gfx->setCursor(textX, 0);
  gfx->setTextSize(2);
  gfx->print(name);
  gfx->setTextSize(1);

  if(subMenuCount > 4) {
    gfx->fillRect(123, 17, 5, 46, COLOR_WHITE);
    int scrollBarLen = 44 - (4 * (subMenuCount - 4));
    gfx->fillRect(124, 18 + (4 * dispOffset), 3, scrollBarLen, COLOR_BLACK);
  }

  for(int i = 0; i < 4 && i < subMenuCount; i++) {
    if(cursorPos == dispOffset + i) {
      if(selectedMenu != nullptr) {
        gfx->fillRect(0, 16 + (12 * i), 122, 12, COLOR_WHITE);
        gfx->setTextColor(COLOR_BLACK);
      }
      else {
        gfx->drawRect(0, 16 + (12 * i), 122, 12, COLOR_WHITE);
      }
    }

    if(subMenus[dispOffset + i]->isMenu()) {
      gfx->setCursor(2, 16 + (12 * i) + 2);
      gfx->print(subMenus[dispOffset + i]->getName());

      int xTop = 116;
      int yTop = 16 + (12 * i) + 2;
      int xBot = xTop;
      int yBot = yTop + 6;
      int xRight = xTop + 3;
      int yRight = yTop + 3;
      gfx->fillTriangle(xTop, yTop, xBot, yBot, xRight, yRight, COLOR_WHITE);
    }
    else {
      subMenus[dispOffset + i]->draw(gfx, 2, 16 + (12 * i) + 2);
    }

    gfx->setTextColor(COLOR_WHITE);
  }
}

void Menu::handleButtons(bool up, bool down, bool left, bool right, bool center, bool upHeld, bool downHeld, bool leftHeld, bool rightHeld) {
  if(selectedMenu != nullptr) { //A menu item is selected or we are in a sub menu
    selectedMenu->handleButtons(up, down, left, right, center, upHeld, downHeld, leftHeld, rightHeld);
    return;
  }
  else if(center && (subMenus[cursorPos]->isBack() || subMenus[cursorPos]->isButton())) {
    subMenus[cursorPos]->handleButtons(false, false, false, false, true, false, false, false, false);
    return;
  }

  if(up) {
    if(cursorPos >= 2 && cursorPos < subMenuCount - 2) {
      dispOffset--;
    }
    
    cursorPos--;

    if(cursorPos < 0) {
      cursorPos = subMenuCount - 1;
      dispOffset = max(subMenuCount - 4, 0);
    }

    markDirty();
  }
  else if(down) {
    cursorPos++;

    if (cursorPos >= subMenuCount) {
      cursorPos = 0;
      dispOffset = 0;
    }
    else if(cursorPos >= 2 && cursorPos < subMenuCount - 2) {
      dispOffset++;
    }

    markDirty();
  }
  else if(center && (subMenus[cursorPos]->isMenu() || ((MenuItem*)subMenus[cursorPos])->isEditable())) {
    selectedMenu = subMenus[cursorPos];
    if(!subMenus[cursorPos]->isMenu()) { ((MenuItem*)subMenus[cursorPos])->select(); }
    markDirty();
  }
}

bool Menu::addItem(AbstractMenu* item) {
  if(subMenuCount >= MAX_SUBMENUS) { return false; }

  item->setParent(this);
  subMenus[subMenuCount] = item;
  subMenuCount++;

  return true;
}

void Menu::close() {
  resetCursor();
  parent->resetSelected();
}

void Menu::resetSelected() {
  if(selectedMenu != nullptr) {
    if(selectedMenu->isMenu()) { ((Menu*)selectedMenu)->resetSelected(); }
    selectedMenu = nullptr;
    markDirty();
  }
}

void Menu::resetCursor() {
  if(selectedMenu != nullptr && selectedMenu->isMenu()) { ((Menu*)selectedMenu)->resetCursor(); }
  cursorPos = 0;
  dispOffset = 0;
}

//MainMenu implementation
bool MainMenu::draw(Adafruit_SSD1306* gfx)  {
    if (show && needsRedraw()) {
        gfx->clearDisplay();
        Menu::draw(gfx, 0, 0);
        gfx->display();
    }
    return show;
}

void MainMenu::open() {
  show = true;
  markDirty();
}

void MainMenu::close() {
  resetCursor();
  show = false;
  markDirty();
}

bool MainMenu::needsRedraw() {
  if(dirty) {
    dirty = false;
    return true;
  }
  return false;
}

//MenuItem implementation
MenuItem::MenuItem(const char* name, const char* valueUnit, AbstractValue* value, bool editable) : AbstractMenu(name), value(value), valueUnit(valueUnit), editable(editable) {}

void MenuItem::draw(Adafruit_SSD1306* gfx, int x, int y) {
  gfx->setCursor(x, y);
  gfx->print(name);
  gfx->print(":");

  int unitLen = strlen(valueUnit);

  value->print(gfx, x + ((int)strlen(name) * 6) + 6, y, unitLen * 6);

  if(unitLen > 0) {
    gfx->setCursor(120 - (strlen(valueUnit) * 6), y);
    gfx->print(valueUnit);
  }
}

void MenuItem::handleButtons(bool up, bool down, bool left, bool right, bool center, bool upHeld, bool downHeld, bool leftHeld, bool rightHeld) {
  if(value->handleButtons(up, down, left, right, upHeld, downHeld, leftHeld, rightHeld)) { markDirty(); }
  if(center) { deselect(); parent->resetSelected(); }
}

void MenuItem::markDirty() { parent->markDirty(); }

//MenuButton implementation
MenuButton::MenuButton(const char* nameStd, const char* nameAlt, bool(*callback)()) : AbstractMenu(nameStd), nameStd(nameStd), nameAlt(nameAlt), callback(callback) {}

void MenuButton::handleButtons(bool up, bool down, bool left, bool right, bool center, bool upHeld, bool downHeld, bool leftHeld, bool rightHeld) {
  if(center) {
    if(callback()) {
      name = (name == nameStd) ? nameAlt : nameStd;
      markDirty();
    }
  }
}

void MenuButton::draw(Adafruit_SSD1306* gfx, int x, int y) {
  gfx->setCursor(x, y);
  gfx->print(getName());
}

void MenuButton::markDirty() { parent->markDirty(); }

void MenuButton::setNameToStd() {
  name = nameStd;
  markDirty();
}

void MenuButton::setNameToAlt() {
  name = nameAlt;
  markDirty();
}

//BackItem implementation
BackItem::BackItem() : BackItem(nullptr) {}

BackItem::BackItem(void (*callback)()) : AbstractMenu("Back"), callback(callback) {}

void BackItem::draw(Adafruit_SSD1306* gfx, int x, int y) {
  gfx->setCursor(x, y);
  gfx->print(getName());
}

void BackItem::handleButtons(bool left, bool right, bool up, bool down, bool center, bool upHeld, bool downHeld, bool leftHeld, bool rightHeld) {
  if(center) {
    if(callback != nullptr) { callback(); }

    parent->close();
  }
}

//BoolValue implementation
BoolValue::BoolValue(bool* value) : value(value) {}

void BoolValue::print(Adafruit_SSD1306* gfx, int x, int y, int xNegOff) {
  const char* s = (interacting ? valueCopy : *value) ? "true" : "false";

  int textX = 120 - xNegOff - ((int)strlen(s) * 6);
  gfx->setCursor(textX, y);
  gfx->print(s);
}

bool BoolValue::handleButtons(bool up, bool down, bool left, bool right, bool upHeld, bool downHeld, bool leftHeld, bool rightHeld) {
  if(left || right) {
    valueCopy = !valueCopy;
    return true;
  }
  return false;
}

//NamedBoolValue implementation
NamedBoolValue::NamedBoolValue(bool* value, const char* trueText, const char* falseText) : value(value), trueText(trueText), falseText(falseText) {}

void NamedBoolValue::print(Adafruit_SSD1306* gfx, int x, int y, int xNegOff) {
  const char* s = (interacting ? valueCopy : *value) ? trueText : falseText;

  int textX = 120 - xNegOff - ((int)strlen(s) * 6);
  gfx->setCursor(textX, y);
  gfx->print(s);
}

bool NamedBoolValue::handleButtons(bool up, bool down, bool left, bool right, bool upHeld, bool downHeld, bool leftHeld, bool rightHeld) {
  if(left || right) {
    valueCopy = !valueCopy;
    return true;
  }
  return false;
}

//IntValue implementation
IntValue::IntValue(int* value, int min, int max, int increment, bool multiIncrement) : value(value), lowerLimit(min), upperLimit(max), increment(increment), multiIncrement(multiIncrement) {}

void IntValue::print(Adafruit_SSD1306* gfx, int x, int y, int xNegOff) {
  char s[10] = {};
  sprintf(s, "%d", interacting ? valueCopy : *value);

  int textX = 120 - xNegOff - ((int)strlen(s) * 6);
  gfx->setCursor(textX, y);
  gfx->print(s);
}

bool IntValue::handleButtons(bool up, bool down, bool left, bool right, bool upHeld, bool downHeld, bool leftHeld, bool rightHeld) {
  if(left) {
    if(valueCopy > lowerLimit) {
      if(downHeld && multiIncrement) {
        valueCopy -= min(increment * 10, valueCopy - lowerLimit);
      }
      else if(upHeld && multiIncrement) {
        valueCopy -= min(increment* 100, valueCopy - lowerLimit);
      }
      else  {
        valueCopy -= increment;
      }
    }

    return true;
  }
  else if(right) {
    if(valueCopy < upperLimit) {
      if(downHeld && multiIncrement) {
        valueCopy += min(increment * 10, upperLimit - valueCopy);
      }
      else if(upHeld && multiIncrement) {
        valueCopy += min(increment * 100, upperLimit - valueCopy);
      }
      else {
        valueCopy += increment;
      }
    }

    return true;
  }
  return false;
}

//UnsignedIntValue implementation
UnsignedIntValue::UnsignedIntValue(unsigned int* value, unsigned int min, unsigned int max, unsigned int increment, bool multiIncrement) :
value(value), lowerLimit(min), upperLimit(max), increment(increment), multiIncrement(multiIncrement) {}

void UnsignedIntValue::print(Adafruit_SSD1306* gfx, int x, int y, int xNegOff) {
  char s[10] = {};
  sprintf(s, "%u", interacting ? valueCopy : *value);

  int textX = 120 - xNegOff - ((int)strlen(s) * 6);
  gfx->setCursor(textX, y);
  gfx->print(s);
}

bool UnsignedIntValue::handleButtons(bool up, bool down, bool left, bool right, bool upHeld, bool downHeld, bool leftHeld, bool rightHeld) {
  if(left) {
    if(valueCopy > lowerLimit) {
      if(downHeld && multiIncrement) {
        valueCopy -= min(increment * 10, valueCopy - lowerLimit);
      }
      else if(upHeld && multiIncrement) {
        valueCopy -= min(increment* 100, valueCopy - lowerLimit);
      }
      else  {
        valueCopy -= increment;
      }
    }

    return true;
  }
  else if(right) {
    if(valueCopy < upperLimit) {
      if(downHeld && multiIncrement) {
        valueCopy += min(increment * 10, upperLimit - valueCopy);
      }
      else if(upHeld && multiIncrement) {
        valueCopy += min(increment * 100, upperLimit - valueCopy);
      }
      else {
        valueCopy += increment;
      }
    }

    return true;
  }
  return false;
}

//NamedIntValue implementation
NamedIntValue::NamedIntValue(unsigned int* value, unsigned int max, const char* texts[], bool rollOver) : value(value), upperLimit(max), texts(texts), rollOver(rollOver) {}

void NamedIntValue::print(Adafruit_SSD1306* gfx, int x, int y, int xNegOff) {
  const char* s = interacting ? texts[valueCopy] : texts[*value];

  int textX = 120 - xNegOff - ((int)strlen(s) * 6);
  gfx->setCursor(textX, y);
  gfx->print(s);
}

bool NamedIntValue::handleButtons(bool up, bool down, bool left, bool right, bool upHeld, bool downHeld, bool leftHeld, bool rightHeld) {
  if(left) {
    if(valueCopy > 0) { valueCopy -= 1; }
    else if(rollOver) { valueCopy = upperLimit; }

    return true;
  }
  else if(right) {
    if(valueCopy < upperLimit) { valueCopy += 1; }
    else if(rollOver) { valueCopy = 0; }

    return true;
  }
  return false;
}

//StringValue implementation
StringValue::StringValue(const char* value) : value(value) {}

void StringValue::print(Adafruit_SSD1306* gfx, int x, int y, int xNegOff) {
  int textX = 120 - xNegOff - ((int)strlen(value) * 6);
  gfx->setCursor(textX, y);
  gfx->print(value);
}
