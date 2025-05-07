/*
 * File:        APP_LED.cpp
 * Author:      Marcus Lechner
 * Created:     2025-03-22
 * Description: Implementation of LED animations using FastLED library
 * License:     Custom MIT License (Non-Commercial + Beerware)
 */

#include "APP_LED.hpp"
#include "APP_TIMER.hpp"
#include <FastLED.h>

namespace
{
    //namespace for private variables and functions
    //anonymous namespace, everything inside this namespace is private to this.cpp file
    //this is a good practice to avoid name collisions and keep the code organized

    //namespace keeps all symbols(functions, variables, constants, class or objects) private and ensurese they all have internal linkcage


    constexpr uint8_t DATA_PIN = 4;
    constexpr uint8_t NUM_LEDS = 24;
    constexpr uint8_t BRIGHTNESS = 32;
    constexpr uint8_t FRAMES_PER_SECOND = 120;
    constexpr uint8_t FRAME_DELAY_MS = (1000 + (FRAMES_PER_SECOND / 2)) / FRAMES_PER_SECOND; //round up to the nearest ms with integer division

    void rainbow();
    void rainbowWithGlitter();
    void confetti();
    void sinelon();
    void juggle();
    void bpm();

    Timer led_timer(FRAME_DELAY_MS, true); // 8ms timer for LED animation

    CRGB leds[NUM_LEDS]; //RGB pixel obkject array, each pixel object has 3 uint8_t values for red, green and blue
    //could just make a struct of a pixel with 3 uint8_t values
    uint8_t gCurrentPattern = 0;
    uint8_t gHue = 0;

    using PatternFn = void (*)();

    //------------typedef vs using and type aliases------------------//
    //PattenFn is a type alias for a function pointer type
    //"using" is a modern C++ keyword that is essentially the replacement for typedef
    //it is more readable and allows for more complex type definitions
    //has template support, so it can be used with templates and other type aliases
    //classically it would be typedef void (*PatternFn)(); but using is more readable and modern C++

    PatternFn gPatterns[] = 
    {
        rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm
    };

    void rainbow()
    {
        fill_rainbow(leds, NUM_LEDS, gHue, 7);
    }

    void rainbowWithGlitter()
    {
        rainbow();
        if (random8() < 80)
        {
            leds[random16(NUM_LEDS)] += CRGB::White;
        }
    }

    void confetti()
    {
        fadeToBlackBy(leds, NUM_LEDS, 10);
        int pos = random16(NUM_LEDS);
        leds[pos] += CHSV(gHue + random8(64), 200, 255);
    }

    void sinelon()
    {
        fadeToBlackBy(leds, NUM_LEDS, 20);
        int pos = beatsin16(13, 0, NUM_LEDS - 1);
        leds[pos] += CHSV(gHue, 255, 192);
    }

    void bpm()
    {
        uint8_t BeatsPerMinute = 62;
        CRGBPalette16 palette = PartyColors_p;
        uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);

        for (int i = 0; i < NUM_LEDS; ++i)
        {
            leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
        }
    }

    void juggle()
    {
        fadeToBlackBy(leds, NUM_LEDS, 20);
        uint8_t dothue = 0;
        for (int i = 0; i < 8; ++i)
        {
            leds[beatsin16(i + 7, 0, NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
            dothue += 32;
        }
    }

    void nextPattern()
    {
        gCurrentPattern = (gCurrentPattern + 1) % (sizeof(gPatterns) / sizeof(gPatterns[0]));
    }
}

void APP_LED::init()
{
    FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
}

void APP_LED::process()
{
    if (led_timer.expired())
    {
        // printf("LED timer expired\n");
        gPatterns[gCurrentPattern]();
        FastLED.show(); //updates fastled interal clock
    }

    EVERY_N_MILLISECONDS(20) 
    { 
        gHue++; 
    }

    EVERY_N_SECONDS(10) 
    { 
        nextPattern(); 
    }
}
