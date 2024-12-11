#ifndef CAPACITIVETOUCHBUTTON_H
#define CAPACITIVETOUCHBUTTON_H

#include <Arduino.h>

namespace CoxBox {
    class CapacitiveTouchButton {
        public:
            CapacitiveTouchButton(int pin);
            void takeReading();
            bool getPressed();

        private:
            int pin;
            int buttonState;
            int lastButtonState = LOW;
            bool pressed = false;

            unsigned long lastDebounceTime = 0;
            const unsigned long debounceDelay = 50;
    };
}

#endif