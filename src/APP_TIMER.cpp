/*
 * File:        APP_TIMER.cpp
 * Author:      Marcus Lechner
 * Created:     2025-03-15
 * Description: Implementation of a non-blocking software timer for embedded applications.
 * License:     Custom MIT License (Non-Commercial + Beerware)
 */

#include "APP_TIMER.hpp"
#include <Arduino.h>

Timer::Timer(unsigned long intervalMs, bool startNow)
    : _interval(intervalMs),
      _lastTime(startNow ? millis() : 0),
      _enabled(startNow)
{}

void Timer::start()
{
    _lastTime = millis();
    _enabled = true;
}

void Timer::stop()
{
    _enabled = false;
}

void Timer::reset()
{
    _lastTime = millis();
}

bool Timer::expired()
{
    if (!_enabled)
        return false;

    unsigned long now = millis();
    if (now - _lastTime >= _interval)
    {
        _lastTime = now;
        return true;
    }

    return false;
}

void Timer::setInterval(unsigned long intervalMs)
{
    _interval = intervalMs;
}

bool Timer::isRunning() const
{
    return _enabled;
}
