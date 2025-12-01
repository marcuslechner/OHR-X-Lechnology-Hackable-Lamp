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
    constexpr int CLOSED_POSITION = 0;   // Servo position for fully closed
    constexpr int OPEN_POSITION = 100;   // Servo position for fully ope
    constexpr int refresh_period = 20; // 50ms refresh period for the servo

    enum State
    {
        IDLE,
        MOVING
    };

    Timer servo_timer(refresh_period, true); // 50ms timer for servo refresh
    Timer servo_wait_timer(1000, true); // 2 second timer for servo wait

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
    int desired_position = OPEN_POSITION/2; // Default to mid position
    int current_position = desired_position; // Default to mid position
}


/**
 *  @brief Sets the desired servo position in percent (0-100)
 *  @param position Desired position in percent (0 = closed, 100 = open)
 */

void APP_SERVO::setPosition(int position)
{
    if(position < CLOSED_POSITION) position = CLOSED_POSITION;
    if(position > OPEN_POSITION) position = OPEN_POSITION;
    desired_position = position;
}

void APP_SERVO::init()
{
    ESP32PWM::allocateTimer(0);
    myservo.setPeriodHertz(50);
    myservo.attach(SERVO_PIN, 500, 2400);
    myservo.write(current_position);
}

void APP_SERVO::process()
{
    static State servo_state = IDLE;

    if(servo_timer.expired()) // Check if the timer has expired)
    {
        servo_timer.start(); // Start the servo refresh timer
        
        switch (servo_state)
        {
            case IDLE:
                if (desired_position != current_position)
                {
                    servo_state = MOVING;
                    // servo_wait_timer.start(); // Start the wait timer when we begin moving
                    // servo_timer.start(); // Start the servo refresh timer
                }
                else
                {   
                    myservo.release(); // Release the servo if at desired position
                    // if(servo_wait_timer.expired())
                    // {
                    //     //make up new position
                    //     desired_position = desired_position + (OPEN_POSITION/2);
                    //     desired_position = desired_position % OPEN_POSITION;
                    // }
                }
                break;

            case MOVING:
                // This section simulates the behavior of a potentiometer using a simple algorithm.
                // It creates a fake ADC value that oscillates between ADC_MIN and ADC_MAX.
                
                static int fake_adc = CLOSED_POSITION;
                static bool rising = true;
                if(desired_position > current_position) 
                {
                    current_position++;
                } 
                else if(desired_position < current_position) 
                {
                    current_position--;
                }
            
                Serial.print("current_position ");
                Serial.println(current_position);
                int val = current_position;
                // -----------------------------------------

                val = map(val, CLOSED_POSITION, OPEN_POSITION, 0, 180);

                Serial.print("val ");
                Serial.println(val);
                myservo.write(val);
                
                if(current_position == desired_position)
                {
                    servo_state = IDLE;
                    // servo_wait_timer.start(); // Start the wait timer when we reach the desired position
                    // servo_timer.stop(); // Stop the servo refresh timer
                }


                break;
        }
    }
}
