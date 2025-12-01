/*
 * File:        APP_LED.hpp
 * Author:      Marcus Lechner
 * Created:     2025-03-22
 * Description: Interface for the LED animation controller using FastLED
 * License:     Custom MIT License (Non-Commercial + Beerware)
 */

#ifndef APP_LED_HPP
#define APP_LED_HPP

#include <stdint.h>

namespace APP_LED
{
    void init();
    void process();
    void setSolidColor(uint8_t r, uint8_t g, uint8_t b);
    void setAnimation(uint8_t animId);
}

#endif // APP_LED_HPP
