/*
 * File:        APP_SERVO.cpp
 * Author:      Marcus Lechner
 * Created:     2025-03-22
 * Description: Servo control implementation with smoothing and auto-release
 * License:     Custom MIT License (Non-Commercial + Beerware)
 */


#include "APP_SERVO.hpp"
#include "APP_TIMER.hpp"
#include <Arduino.h>
#include <ESP32Servo.h>

#define TEST_MODE 1  // Set to 0 to disable test mode

namespace //unamed (anonymous) namespace, everything inside this namespace is private to this.cpp file
{  
    constexpr int SERVO_PIN = 18;
    constexpr int POT_PIN = 34;
    constexpr int ADC_MAX = 4096;
    constexpr int ADC_MIN = 500;  
    constexpr int refresh_period = 20; // 50ms refresh period for the servo

    Timer servo_timer(refresh_period, true); // 50ms timer for servo refresh

    ///------------about constexpr: qualifiers and specifiers------------------///
    //constexpr is known as a compile-time constant, it is evaluated at compile time and can be used in switch statements, array sizes, etc.
    //constexpr is a type-safe alternative to #define, it is scoped and can be used in templates, constexpr int = 5; is a compile-time constant
    //constexpr replaces #define, it can have a datatype assigned to it which is good for static analysis
    //constexpr replaces const, it is evaluated at compile time and guarantees the value, const int = analog_read() can be any value 
    //but stays constant after declaration 
    //constexpr fully and safely replace #define

    //const is a qualifier because it says what can be changed and how, examples const, and volitie

    //static is a specifier that tells the compiler how to interpret or allocate something, it can be used for variables, functions, and classes
    //what is this datatypes role or behavior

    //const is a qualifier because it says what and when a datatypoe can be changed,
    // const int = 5; can never be changed, const int = analog_read() can be any value but stays constant after declaration

    //constexpr is  a specifier because it specifies to the compiler what the behavior of the data type is, ie it cannot
    //be seen outside of this file

    Servo myservo;
    int previous_val = 0;
    int steps_til_release = 0;
}

void APP_SERVO::init()
{
    ESP32PWM::allocateTimer(0);
    myservo.setPeriodHertz(50);
    myservo.attach(SERVO_PIN, 500, 2400);
}

void APP_SERVO::process()
{
    if(servo_timer.expired()) // Check if the timer has expired)
    {

#if TEST_MODE
        // --- TEST MODE: Simulated analog input ---
        // This section simulates the behavior of a potentiometer using a simple algorithm.
        // It creates a fake ADC value that oscillates between ADC_MIN and ADC_MAX.
        
        static int fake_adc = ADC_MIN;
        static bool rising = true;

        if (rising) {
            fake_adc += 60;
            if (fake_adc >= ADC_MAX - ADC_MIN) rising = false;
        } else {
            fake_adc -= 60;
            if (fake_adc <= ADC_MIN) rising = true;
        }

        int val = fake_adc;
        // -----------------------------------------
#else       
        int val = analogRead(POT_PIN); //TODO: add lowpass filter to the analog read value
        // int val = 4096/2;
#endif
        val = map(val, ADC_MIN, ADC_MAX - ADC_MIN, 90, 150);

        Serial.print("val ");
        Serial.println(val);

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
                myservo.release();
                //TODO: this will just count up to 20 then reset the counter
                steps_til_release = 0; // Reset the counter after releasing
                Serial.println("Servo released due to inactivity.");
            }
        }
    }
}
