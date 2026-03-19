#include <Arduino.h>
#include "debugging.h"

void led_blink(int led_pin) {
    pinMode(led_pin, OUTPUT);

    while (true) {
        digitalWrite(led_pin, HIGH);
        delay(1000);
        digitalWrite(led_pin, LOW);
        delay(1000);
    }
}