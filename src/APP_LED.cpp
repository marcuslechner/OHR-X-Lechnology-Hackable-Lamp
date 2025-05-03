/*
 * File:        APP_LED.cpp
 * Author:      Marcus Lechner
 * Created:     2025-03-22
 * Description: Implementation of LED animations using FastLED library
 * License:     Custom MIT License (Non-Commercial + Beerware)
 */

 #include "APP_LED.hpp"
#include <FastLED.h>

namespace
{
    constexpr int DATA_PIN = 4;
    constexpr int NUM_LEDS = 24;
    constexpr int BRIGHTNESS = 255;
    constexpr int FRAMES_PER_SECOND = 120;

    void rainbow();
    void rainbowWithGlitter();
    void confetti();
    void sinelon();
    void juggle();
    void bpm();

    CRGB leds[NUM_LEDS];
    uint8_t gCurrentPattern = 0;
    uint8_t gHue = 0;

    using PatternFn = void (*)();
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

void APP_LED::begin()
{
    FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
}

void APP_LED::update()
{
    gPatterns[gCurrentPattern]();
    FastLED.show();
    FastLED.delay(1000 / FRAMES_PER_SECOND);

    EVERY_N_MILLISECONDS(20) { gHue++; }
    EVERY_N_SECONDS(10) { nextPattern(); }
}
