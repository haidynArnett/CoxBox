#include <capacitiveTouchButton.h>

namespace CoxBox {
    CapacitiveTouchButton::CapacitiveTouchButton(int pin) {
        this->pin = pin;
    }

    void CapacitiveTouchButton::takeReading() {
        pinMode(this->pin, OUTPUT);
        digitalWrite(this->pin, LOW);
        delayMicroseconds(50);
        unsigned long start = micros();
        pinMode(this->pin, INPUT);
        int reading = LOW;
        while (digitalRead(this->pin) != 1) {
            if (micros() - start > 20) {
                reading = HIGH;
                break;
            }
        }

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

    bool CapacitiveTouchButton::getPressed() {
        if (this->pressed) {
            this->pressed = false;
            return true;
        }
        return false;
    }
}