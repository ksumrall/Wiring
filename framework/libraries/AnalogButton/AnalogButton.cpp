/*
||
|| @author         Alexander Brevig <abrevig@wiring.org.co>
|| @url            http://wiring.org.co/
|| @url            http://alexanderbrevig.com/
|| @contribution   Brett Hagman <bhagman@wiring.org.co>
|| @contribution   https://github.com/cyborgsimon
|| @contribution   Chris van Marle
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

#include <Wiring.h>
#include "Button.h"

#define CURRENT 0
#define PREVIOUS 1
#define CHANGED 2

#define SAMPLE 3
#define CLICKSENT 4

#define DEFAULT_HOLDEVENTTHRESHOLDTIME 500

/*
|| @constructor
|| | Set the initial state of this button
|| #
||
|| @parameter lowValue is the low value in the range for analog value
|| @parameter highValue is the high value in the analog range for this button
*/
Button::Button(uint8_t lowValue, uint8_t highValue)
{
    ID = 0;

    lowValue = lowValue;
    highValue = highValue;

    state = 0;
    bitWrite(state, CURRENT, false);
    bitWrite(state, PREVIOUS, false);
    bitWrite(state, CHANGED, false);
    bitWrite(state, SAMPLE, mode);
    bitWrite(state, CLICKSENT, true);

    pressedStartTime = 0;
    debounceDelayTime = 50;
    debounceStartTime = 0;
    lastReleaseTime = 0;
    multiClickThresholdTime = 0;

    holdEventThresholdTime = 0;
    holdEventRepeatTime = 0;

    cb_onPress = 0;
    cb_onRelease = 0;
    cb_onClick = 0;
    cb_onHold = 0;

    numberOfPresses = 0;
    triggeredHoldEvent = true;
}

/*
|| @description
|| | Return true if the button has been pressed
|| #
*/
bool Button::isPressed(void) const
{
    return bitRead(state, CURRENT);
}

/*
|| @description
|| | Return true if state has been changed
|| #
*/
bool Button::stateChanged(void) const
{
    return bitRead(state, CHANGED);
}

/*
|| @description
|| | Return true if the button is pressed, and was not pressed before
|| #
*/
bool Button::uniquePress(void) const
{
    return (isPressed() && stateChanged());
}

/*
|| @description
|| | Return > 0 if the button is clicked, or 0 if not.
|| #
*/
unsigned int Button::clicked(void)
{
    if (bitRead(state, CLICKSENT) == false &&
            bitRead(state, CURRENT) == false &&
            ((multiClickThresholdTime == 0) ||                                // We don't want multiClicks OR
                ((millis() - lastReleaseTime) > multiClickThresholdTime)))       // we are outside of our multiClick threshold time.
    {
        bitWrite(state, CLICKSENT, true);
        return clickCount;
    }
    return 0;
}


/*
|| @description
|| | onHold polling model
|| | Check to see if the button has been pressed for time ms
|| | This is a unique value - this method will return false if it is called
|| | a second time while the button is still held.
|| #
*/
bool Button::held(unsigned long time /*=0*/)
{
    unsigned long threshold = time ? time : holdEventThresholdTime; //use holdEventThreshold if time == 0
    //should we trigger a onHold event?
    if (isPressed() && !triggeredHoldEvent)
    {
        if (millis() - pressedStartTime > threshold)
        {
            triggeredHoldEvent = true;
            return true;
        }
    }
    return false;
}

/*
|| @description
|| | Polling model for holding, this is true every check after hold time
|| | Check to see if the button has been pressed for time ms
|| #
*/
bool Button::heldFor(unsigned long time) const
{
    if (isPressed())
    {
        if (millis() - pressedStartTime > time)
        {
            return true;
        }
    }
    return false;
}

/*
|| @description
|| | Set the debounce delay time
|| #
*/
void Button::setDebounceDelay(unsigned int debounceDelay)
{
    debounceDelayTime = debounceDelay;
}

/*
|| @description
|| | Set the hold time threshold
|| #
*/
void Button::setHoldThreshold(unsigned int holdTime)
{
    holdEventThresholdTime = holdTime;
}

/*
|| @description
|| | Set the hold repeat time
|| #
*/
void Button::setHoldRepeat(unsigned int repeatTime)
{
    holdEventRepeatTime = repeatTime;
}

/*
|| @description
|| | Set the multi click time threshold
|| #
*/
void Button::setMultiClickThreshold(unsigned int multiClickTime)
{
    multiClickThresholdTime = multiClickTime;
}

/*
|| @description
|| | Register a handler for presses on this button
|| #
||
|| @parameter handler The function to call when this button is pressed
*/
void Button::pressHandler(buttonEventHandler handler)
{
    cb_onPress = handler;
}

/*
|| @description
|| | Register a handler for releases on this button
|| #
||
|| @parameter handler The function to call when this button is released
*/
void Button::releaseHandler(buttonEventHandler handler)
{
    cb_onRelease = handler;
}

/*
|| @description
|| | Register a handler for clicks on this button
|| #
||
|| @parameter handler The function to call when this button is clicked
*/
void Button::clickHandler(buttonEventHandler handler)
{
    cb_onClick = handler;
}

/*
|| @description
|| | Register a handler for when this button is held
|| #
||
|| @parameter handler The function to call when this button is held
|| @optionalparameter holdTime Sets the hold time for the handler. If 0, then the default hold time is used.
*/
void Button::holdHandler(buttonEventHandler handler, unsigned long holdTime /*=0*/)
{
    setHoldThreshold(holdTime ? holdTime : DEFAULT_HOLDEVENTTHRESHOLDTIME);

    cb_onHold = handler;
}

/*
|| @description
|| | Get the time this button has been held
|| #
||
|| @return The time this button has been held
*/
unsigned long Button::holdTime() const
{
    if (isPressed())
    {
        return millis() - pressedStartTime;
    }
    else return 0;
}

/*
|| @description
|| | Get the time this button had been held.
|| #
||
|| @return The time this button had been held
*/
unsigned long Button::heldTime() const
{
    return millis() - pressedStartTime;
}

/*
|| @description
|| | Compare a button object against this
|| #
||
|| @parameter  rhs the Button to compare against this Button
||
|| @return true if they are the same
*/
bool Button::operator==(Button &rhs)
{
    return (this == &rhs);
}
