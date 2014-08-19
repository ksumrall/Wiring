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

