/*
 * File:        main.cpp
 * Author:      Marcus Lechner
 * Created:     2025-03-22
 * Description: Main Application code
 * License:     Custom MIT License (Non-Commercial + Beerware)

    Copyright (c) [2025] [Marcus Lechner]

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
    to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
    and/or sell copies of the Software, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
    The Software may NOT be used for commercial purposes.
    If you find this software useful and we meet someday, you can buy me a beer (or a beverage of your choice) in return.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
    FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES, OR 
    OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
    DEALINGS IN THE SOFTWARE.

 */


#include <Arduino.h>
#include "APP_LED.hpp"
#include "APP_SERVO.hpp"
#include "APP_TIMER.hpp"
#include "APP_BLINKY.hpp"
#include "APP_BLE.hpp"




void setup()
{
    delay(3000);
    Serial.begin(115200);
    delay(1000);
    Serial.println("Starting up...");
    APP_BLINKY::init();   
    APP_BLE::init();
    // APP_LED::init();
    APP_SERVO::init();

}

void loop()
{
    APP_BLINKY::process();
    APP_BLE::process();
    // APP_LED::process();
    APP_SERVO::process();
}