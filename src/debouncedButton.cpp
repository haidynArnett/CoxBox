#include <Arduino.h>
#include "../include/debouncedButton.hpp"

namespace CoxBox {

    DebouncedButton::DebouncedButton(int pin) {
        this->pin = pin;
        pinMode(pin, INPUT);
    }

    void DebouncedButton::takeReading() {
        int reading = digitalRead(this->pin);
        if (reading != this->lastButtonState) {
            this->lastDebounceTime = millis();
        }

        if ((millis() - this->lastDebounceTime) > debounceDelay) {
            if (reading != this->buttonState) {
                this->buttonState = reading;

                if (this->buttonState == 1) {
                    this->pressed = true;
                }
            }
        }
        this->lastButtonState = reading;
    }

    bool DebouncedButton::getPressed() {
        if (this->pressed) {
            this->pressed = false;
            return true;
        }
        return false;
    }
}