

#include "APP_BLINKY.hpp"
#include "APP_TIMER.hpp"
#include <Arduino.h>

namespace
{
    const int LED_PIN = 2;
    const int BLINK_INTERVAL = 500; // in milliseconds
    unsigned long previousMillis = 0;
    bool ledState = false;
    Timer blink_timer(1000,true);


}

void APP_BLINKY::init()
{
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW); // Ensure LED is off at startup
}

void APP_BLINKY::process()
{
    if (blink_timer.expired())
    {
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState ? HIGH : LOW);
    }
}