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
#include "AnalogButtonManager.h"

/*
|| @constructor
|| | Set the initial state of this button
|| #
||
*/
AnalogButtonManager::AnalogButtonManager()
{
}

/*
|| @description
|| | Adds a new button reading to the list of button ranges being managed.
|| #
||
|| @parameter buttonName The name of the button range to manage
|| @parameter lowValue The low value ofthe analogRead
|| @parameter highValue The high value ofthe analogRead
||
|| @returns true if button is pressed.
*/
void AnalogButtonManager::addButon(const char *buttonName, int lowValue, int highValue)
{
}

/*
|| @description
|| | Scan the button and update state and fire events.
|| #
||
|| @returns true if button is pressed.
*/
bool AnalogButtonManager::scan(void)
{
    unsigned long now = millis();
    int sample = digitalRead(pin);

    if (sample != bitRead(state, SAMPLE))
    debounceStartTime = now;                                            // Invalidate debounce timer (i.e. we bounced)

    bitWrite(state, SAMPLE, sample);                                      // Store the sample.

    bitWrite(state, CHANGED, false);

    // If our samples have outlasted our debounce delay (i.e. stabilized),
    // then we can switch state.
    if ((now - debounceStartTime) > debounceDelayTime)
    {
        // Save the previous value
        bitWrite(state, PREVIOUS, bitRead(state, CURRENT));

        // Get the current status of the pin, and normalize into state variable.
        if (sample == mode)
        {
            //currently the button is not pressed
            bitWrite(state, CURRENT, false);
        }
        else
        {
            //currently the button is pressed
            bitWrite(state, CURRENT, true);
        }

        //handle state changes
        if (bitRead(state, CURRENT) != bitRead(state, PREVIOUS))
        {
            //note that the state changed
            bitWrite(state, CHANGED, true);
            // Reset the hold event
            triggeredHoldEvent = false;

            //the state changed to PRESSED
            if (bitRead(state, CURRENT) == true)
            {
                holdEventPreviousTime = 0; // reset hold event
                holdEventRepeatCount = 0;
                numberOfPresses++;

                // If we have another click within the multiClick threshold,
                // increase our click count, otherwise reset.
                if ((now - lastReleaseTime) < multiClickThresholdTime)
                clickCount++;
                else
                clickCount = 1;

                if (cb_onPress)
                cb_onPress(*this);                                            // Fire the onPress event

                pressedStartTime = millis();                                    // Start timing
            }
            else //the state changed to RELEASED
            {
                if (cb_onRelease)
                cb_onRelease(*this);                                          // Fire the onRelease event

                lastReleaseTime = now;
                bitWrite(state, CLICKSENT, false);
            }
        }
        else if (bitRead(state, CURRENT))                                   // if we are pressed...
        {
            if (holdEventThresholdTime > 0 &&                                 // The owner wants hold events, AND
                    ((holdEventPreviousTime == 0) ||                              // (we haven't sent the event yet OR
                        ((holdEventRepeatTime > 0) &&                                // the owner wants repeats, AND
                            ((now - holdEventPreviousTime) > holdEventRepeatTime))) &&  // or it's time for another), AND
                    ((now - pressedStartTime) > holdEventThresholdTime) &&        // we have waited long enough, AND
                    (cb_onHold != NULL))                                          // someone is actually listening.
            {
                cb_onHold(*this);
                holdEventPreviousTime = now;
                holdEventRepeatCount++;
                triggeredHoldEvent = true;
            }
        }
    }

    // Manage the onClick handler
    if (cb_onClick)
    {
        if (bitRead(state, CLICKSENT) == false &&
                bitRead(state, CURRENT) == false &&
                ((multiClickThresholdTime == 0) ||                              // We don't want multiClicks OR
                    ((now - lastReleaseTime) > multiClickThresholdTime)))          // we are outside of our multiClick threshold time.
        {
            cb_onClick(*this);                                                // Fire the onClick event.
            bitWrite(state, CLICKSENT, true);
        }
    }

    return bitRead(state, CURRENT);
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
