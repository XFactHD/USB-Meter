#ifndef MENU_H
#define MENU_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>

//Colors copied from Adafruit_SSD1306.h
#define COLOR_BLACK 0
#define COLOR_WHITE 1

#define MAX_SUBMENUS 14

class AbstractMenu;
class Menu;

//Abstract classes
class AbstractMenu {
public:
    explicit AbstractMenu(const char* name) : AbstractMenu(name, name) {}
    AbstractMenu(const char* name, const char* shortName) : name(name), shortName(shortName) {}
    virtual void handleButtons(bool up, bool down, bool left, bool right, bool center, bool upHeld, bool downHeld, bool leftHeld, bool rightHeld) = 0;
    virtual void draw(Adafruit_SSD1306* gfx, int x, int y) = 0;
    virtual bool isMenu() = 0;
    virtual bool isButton() = 0;
    virtual bool isBack() = 0;
    virtual bool isItem() = 0;
    virtual void markDirty() = 0;

    virtual const char* getName() { return name; }
    const char* getShortName() { return shortName; }
    void setParent(Menu* menu) { parent = menu; }

protected:
    const char* name;
    const char* shortName;
    Menu* parent = nullptr;
};

class AbstractValue {
public:
    virtual void print(Adafruit_SSD1306* gfx, int x, int y, int xOffset) = 0;
    virtual bool handleButtons(bool up, bool down, bool left, bool right, bool upHeld, bool downHeld, bool leftHeld, bool rightHeld) = 0;
    virtual void startInteraction() = 0;
    virtual void endInteraction() = 0;

protected:
    bool interacting = false;
};

//Implementing classes
class MenuItem : public AbstractMenu {
public:
    MenuItem(const char* name, const char* valueUnit, AbstractValue* value, bool editable);
    void handleButtons(bool up, bool down, bool left, bool right, bool center, bool upHeld, bool downHeld, bool leftHeld, bool rightHeld) override;
    void draw(Adafruit_SSD1306* gfx, int x, int y) override;
    bool isMenu() override { return false; }
    bool isButton() override { return false; }
    bool isBack() override { return false; }
    bool isItem() override { return true; }
    void markDirty() override;
    void select() { value->startInteraction(); }
    void deselect() { value->endInteraction(); }
    static bool isSelected() { return false; }
    bool isEditable() const { return editable; }

private:
    AbstractValue* value;
    const char* valueUnit;
    bool editable;

    virtual void resetSelected() {}
};

class MenuButton : public AbstractMenu {
public:
    MenuButton(const char* nameStd, const char* nameAlt, bool (*callback)());
    void handleButtons(bool up, bool down, bool left, bool right, bool center, bool upHeld, bool downHeld, bool leftHeld, bool rightHeld) override;
    bool isMenu() override { return false; }
    bool isButton() override { return true; }
    bool isBack() override { return false; }
    bool isItem() override { return false; }
    void markDirty() override;
    void draw(Adafruit_SSD1306* gfx, int x, int y) override;
    const char* getName() override { return nameStd; }
    void setNameToStd();
    void setNameToAlt();

private:
    const char* nameStd;
    const char* nameAlt;
    bool (*callback)();

    virtual void resetSelected() {}
};

class BackItem : public AbstractMenu {
public:
    BackItem();
    explicit BackItem(void (*callback)());
    void handleButtons(bool up, bool down, bool left, bool right, bool center, bool upHeld, bool downHeld, bool leftHeld, bool rightHeld) override;
    void draw(Adafruit_SSD1306* gfx, int x, int y) override;
    bool isMenu() override { return false; }
    bool isButton() override { return false; }
    bool isBack() override { return true; }
    bool isItem() override { return false; }
    void markDirty() override {}

private:
    virtual void resetSelected() {}
    void (*callback)();
};

class Menu : public AbstractMenu {
public:
    explicit Menu(const char* name) : AbstractMenu(name) {}
    Menu(const char* name, const char* shortName) : AbstractMenu(name, shortName) {}
    void handleButtons(bool up, bool down, bool left, bool right, bool center, bool upHeld, bool downHeld, bool leftHeld, bool rightHeld) override;
    void draw(Adafruit_SSD1306* gfx, int x, int y) override;
    bool isMenu() override { return true; }
    bool isButton() override { return false; }
    bool isBack() override { return false; }
    bool isItem() override { return false; }
    void markDirty() override { parent->markDirty(); }
    virtual void close();
    virtual void resetSelected();
    virtual void resetCursor();
    bool addItem(AbstractMenu* item);

protected:
    AbstractMenu* selectedMenu = nullptr;
    AbstractMenu* subMenus[MAX_SUBMENUS] = {};
    int subMenuCount = 0;
    int cursorPos = 0;
    int dispOffset = 0;
};

class MainMenu : public Menu {
public:
    explicit MainMenu(const char* name) : Menu(name) {}
    MainMenu(const char* name, const char* shortName) : Menu(name, shortName) {}
    bool draw(Adafruit_SSD1306* gfx);
    void markDirty() override { dirty = true; }
    void open();
    void close() override;
    bool isOpen() const { return show; }
    bool needsRedraw();

private:
    void draw(Adafruit_SSD1306* gfx, int x, int y) override { Menu::draw(gfx, x, y); }
    bool show = false;
    bool dirty = false;
};

class BoolValue : public AbstractValue {
public:
    explicit BoolValue(bool* value);
    void print(Adafruit_SSD1306* gfx, int x, int y, int xNegOff) override;
    bool handleButtons(bool up, bool down, bool left, bool right, bool upHeld, bool downHeld, bool leftHeld, bool rightHeld) override;
    void startInteraction() override { valueCopy = (*value); interacting = true; }
    void endInteraction() override { (*value) = valueCopy; interacting = false; }

private:
    bool* value;
    bool valueCopy = false;
};

class NamedBoolValue : public AbstractValue {
public:
    NamedBoolValue(bool* value, const char* trueText, const char* falseText);
    void print(Adafruit_SSD1306* gfx, int x, int y, int xNegOff) override;
    bool handleButtons(bool up, bool down, bool left, bool right, bool upHeld, bool downHeld, bool leftHeld, bool rightHeld) override;
    void startInteraction() override { valueCopy = (*value); interacting = true; }
    void endInteraction() override { (*value) = valueCopy; interacting = false; }

private:
    bool* value;
    bool valueCopy = false;
    const char* trueText;
    const char* falseText;
};

class IntValue : public AbstractValue {
public:
    IntValue(int* value, int min, int max, int increment = 1, bool multiIncrement = false);
    void print(Adafruit_SSD1306* gfx, int x, int y, int xNegOff) override;
    bool handleButtons(bool up, bool down, bool left, bool right, bool upHeld, bool downHeld, bool leftHeld, bool rightHeld) override;
    void startInteraction() override { valueCopy = (*value); interacting = true; }
    void endInteraction() override { (*value) = valueCopy; interacting = false; }

private:
    int* value;
    int valueCopy = 0;
    int lowerLimit;
    int upperLimit;
    int increment;
    bool multiIncrement;
};

class UnsignedIntValue : public AbstractValue {
public:
    //UnsignedIntValue(unsigned int* value, unsigned int min, unsigned int max, bool multiIncrement);
    UnsignedIntValue(unsigned int* value, unsigned int min, unsigned int max, unsigned int increment = 1, bool  multiIncrement = false);
    void print(Adafruit_SSD1306* gfx, int x, int y, int xNegOff) override;
    bool handleButtons(bool up, bool down, bool left, bool right, bool upHeld, bool downHeld, bool leftHeld, bool rightHeld) override;
    void startInteraction() override { valueCopy = (*value); interacting = true; }
    void endInteraction() override { (*value) = valueCopy; interacting = false; }

private:
    unsigned int* value;
    unsigned int valueCopy = 0;
    unsigned int lowerLimit;
    unsigned int upperLimit;
    unsigned int increment;
    bool multiIncrement;
};

class NamedIntValue : public AbstractValue {
public:
    NamedIntValue(unsigned int* value, unsigned int max, const char* texts[], bool rollOver);
    void print(Adafruit_SSD1306* gfx, int x, int y, int xNegOff) override;
    bool handleButtons(bool up, bool down, bool left, bool right, bool upHeld, bool downHeld, bool leftHeld, bool rightHeld) override;
    void startInteraction() override { valueCopy = (*value); interacting = true; }
    void endInteraction() override { (*value) = valueCopy; interacting = false; }

private:
    unsigned int* value;
    unsigned int valueCopy = 0;
    unsigned int upperLimit;
    const char** texts;
    bool rollOver;
};

class StringValue : public AbstractValue {
public:
    explicit StringValue(const char* value);
    void print(Adafruit_SSD1306* gfx, int x, int y, int xNegOff) override;
    bool handleButtons(bool up, bool down, bool left, bool right, bool upHeld, bool downHeld, bool leftHeld, bool rightHeld) override { return false; }
    void startInteraction() override {}
    void endInteraction() override {}

private:
    const char* value;
};

#endif