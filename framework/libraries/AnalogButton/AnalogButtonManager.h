/*
||
|| @author         Alexander Brevig <abrevig@wiring.org.co>
|| @url            http://wiring.org.co/
|| @url            http://alexanderbrevig.com/
|| @contribution   Brett Hagman <bhagman@wiring.org.co>
|| @contribution   https://github.com/cyborgsimon
|| @contribution   Chris van Marle (DebounceButton Library)
||
|| @description
|| | Hardware Abstraction Library for Buttons.
|| | It provides an easy way of handling buttons.
|| |
|| | Wiring Cross-platform Library
|| #
||
|| @license Please see cores/Common/License.txt.
||
*/

#ifndef ANALOGBUTTONMANAGER_H
#define ANALOGBUTTONMANAGER_H

#include <stdint.h>

class AnalogButton;
typedef void (*buttonEventHandler)(AnalogButton&);

struct AnalogButtonStruct
{
    const char *buttonName,
    const int analogReadingLowValue,
    const int analogReadingHighValue
};

class AnalogButtonManager
{
  public:

    // Constructor
    AnalogButtonManager(;

    // Public Member
    uint8_t ID;

    // Methods
    void addButon(AnalogButtonStruct &analogButtonStruct);
    bool scan();
    bool isPressed() const;
    bool stateChanged() const;
    bool uniquePress() const;
    unsigned int clicked();
    bool held(unsigned long time = 0);
    bool heldFor(unsigned long time) const;
    
    // Properties
    void setDebounceDelay(unsigned int debounceDelay);
    void setHoldThreshold(unsigned int holdTime);
    void setHoldRepeat(unsigned int repeatTime);
    void setMultiClickThreshold(unsigned int multiClickTime);
    void pressHandler(const char *buttonName, buttonEventHandler handler);
    void releaseHandler(const char *buttonName, buttonEventHandler handler);
    void clickHandler(const char *buttonName, buttonEventHandler handler);
    void holdHandler(const char *buttonName, buttonEventHandler handler, unsigned long holdTime = 0);

    unsigned long holdTime() const;
    unsigned long heldTime() const;
    inline unsigned long presses() const
    {
      return numberOfPresses;
    }

    inline unsigned int getClickCount() const
    {
      return clickCount;
    }

    inline unsigned int getHoldRepeatCount() const
    {
      return holdEventRepeatCount;
    }

    bool operator==(Button &rhs);

  private:
    uint8_t mode;
    uint8_t state;
    
    struct AnalogButtonStructList
    {
        const char *buttonName,
        const int analogReadingLowValue,
        const int analogReadingHighValue
        AnalogButtonStruct *nx
    }
};

#endif
// ANALOGBUTTONMANAGER_H
