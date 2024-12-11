#ifndef DEBOUNCEDBUTTON_H
#define DEBOUNCEDBUTTON_H

#include <Arduino.h>

namespace CoxBox {
    class DebouncedButton {
        public:
            DebouncedButton(int pin);
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