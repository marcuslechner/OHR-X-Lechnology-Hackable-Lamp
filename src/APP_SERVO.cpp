/*
 * File:        APP_SERVO.cpp
 * Author:      Marcus Lechner
 * Created:     2025-03-22
 * Description: Servo control implementation with smoothing and auto-release
 * License:     Custom MIT License (Non-Commercial + Beerware)
 */


#include "APP_SERVO.hpp"
#include <Arduino.h>
#include <ESP32Servo.h>

namespace //unamed (anonymous) namespace, everything inside this namespace is private to this.cpp file
{  
    constexpr int SERVO_PIN = 18;
    constexpr int POT_PIN = 34;
    constexpr int ADC_MAX = 4096;
    //constexpr replaces #define, it can have a datatype assigned to it which is good for static analysis
    //constexpr replaces const, it is evaluated at compile time and guarantees the value, const int = analog_read() can be any value 
    //but stays constant after declaration 
    //constexpr fully and safely replace #define

    Servo myservo;
    int previous_val = 0;
    int steps_til_release = 0;
}

void APP_SERVO::begin()
{
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    myservo.setPeriodHertz(50);
    myservo.attach(SERVO_PIN, 500, 2400);
}

void APP_SERVO::update()
{
    int val = analogRead(POT_PIN);
    val = map(val, 250, ADC_MAX - 250, 60, 100);

    Serial.print("val ");
    Serial.println(val);

    myservo.write(val);

    delay(100); // can later be refactored for non-blocking

    if (abs(val - previous_val) > 1)
    {
        myservo.write(val);
        previous_val = val;
        steps_til_release = 0;
    }
    else
    {
        steps_til_release++;
        if (steps_til_release > 20)
        {
            myservo.detach();
        }
    }
}
