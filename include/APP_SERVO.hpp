/*
 * File:        APP_SERVO.hpp
 * Author:      Marcus Lechner
 * Created:     2025-03-22
 * Description: Interface for servo motor control using potentiometer input
 * License:     Custom MIT License (Non-Commercial + Beerware)
 */

#ifndef APP_SERVO_HPP
#define APP_SERVO_HPP

namespace APP_SERVO //namespace named APP_SERVO, allows unambiguous calls of begin and update, can have multiple function of begin() accross multiple header files
{ //alternative to a name space would be APP_SERVO_begin(), instead we call the namespace function APP_SERVO::begin()
    //adds structure to the state machine
    void begin();
    void update();
}

#endif // APP_SERVO_HPP
