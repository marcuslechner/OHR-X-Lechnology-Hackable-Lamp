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

    //namespace keeps all symbols(functions, variables, constants, class or objects) private 
    //and ensures they all have internal linkage


    constexpr uint8_t DATA_PIN = 4;
    constexpr uint8_t NUM_LEDS = 30;
    constexpr uint8_t BRIGHTNESS = 255;
    constexpr uint8_t FRAMES_PER_SECOND = 120;
    constexpr uint8_t FRAME_DELAY_MS = (1000 + (FRAMES_PER_SECOND / 2)) / FRAMES_PER_SECOND; //round up to the nearest ms with integer division

    void solidColor();
    void rainbow();
    void rainbowWithGlitter();
    void confetti();
    void sinelon();
    void juggle();
    void bpm();
    void fire();
    void twinkle();
    void cylon();
    void lightning();
    void colorWaves();
    void noisePerlin();

    // extra state for Cylon
    int16_t gCylonPos = 0;
    int8_t  gCylonDir = 1;

    Timer led_timer(FRAME_DELAY_MS, true); // 8ms timer for LED animation

    CRGB leds[NUM_LEDS]; //RGB pixel obkject array, each pixel object has 3 uint8_t values for red, green and blue
    //could just make a struct of a pixel with 3 uint8_t values

    // Solid color used by "Solid Color" pattern
    CRGB   gSolidColor = CRGB::White;

    uint8_t gCurrentPattern = 0;
    uint8_t gHue = 0;

    using PatternFn = void (*)();

    //------------typedef vs using and type aliases------------------//
    //PattenFn is a type alias for a function pointer type
    //"using" is a modern C++ keyword that is essentially the replacement for typedef
    //it is more readable and allows for more complex type definitions
    //has template support, so it can be used with templates and other type aliases
    //classically it would be typedef void (*PatternFn)(); but using is more readable and modern C++


    // Order must match the Flutter app list exactly:
    //
    //  0: Solid Color
    //  1: Rainbow
    //  2: Rainbow w/ Glitter
    //  3: Confetti
    //  4: Sinelon
    //  5: BPM
    //  6: Juggle
    //  7: Fire
    //  8: Twinkle
    //  9: Cylon / Larson Scanner
    // 10: Lightning
    // 11: Color Waves
    // 12: Noise / Perlin
    //
    PatternFn gPatterns[] =
    {
        solidColor,
        rainbow,
        rainbowWithGlitter,
        confetti,
        sinelon,
        bpm,
        juggle,
        fire,
        twinkle,
        cylon,
        lightning,
        colorWaves,
        noisePerlin
    };

    const uint8_t NUM_PATTERNS =  static_cast<uint8_t>(sizeof(gPatterns) / sizeof(gPatterns[0]));

    // ---------------- Pattern implementations ---------------- //


    void solidColor()
    {
        fill_solid(leds, NUM_LEDS, gSolidColor);
    }

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

    void fire()
    {
        // simple ember-like fire: reds/oranges that flicker
        fadeToBlackBy(leds, NUM_LEDS, 40);

        const uint8_t sparks = NUM_LEDS / 3;
        for (uint8_t i = 0; i < sparks; ++i)
        {
            uint8_t pos = random8(NUM_LEDS);
            uint8_t heat = random8(160, 255);
            leds[pos] += CRGB(heat, heat / 4, 0); // orange-ish
        }
    }

    void twinkle()
    {
        // dark background with occasional white-ish twinkles
        fadeToBlackBy(leds, NUM_LEDS, 10);

        if (random8() < 40)
        {
            uint8_t pos = random8(NUM_LEDS);
            leds[pos] = CHSV(gHue + random8(64), 0, 255); // mostly white / pastel
        }
    }

    void cylon()
    {
        // single red "eye" scanning back and forth
        fadeToBlackBy(leds, NUM_LEDS, 20);

        gCylonPos += gCylonDir;

        if (gCylonPos <= 0)
        {
            gCylonPos = 0;
            gCylonDir = 1;
        }
        else if (gCylonPos >= NUM_LEDS - 1)
        {
            gCylonPos = NUM_LEDS - 1;
            gCylonDir = -1;
        }

        // eye + a little tail
        leds[gCylonPos] = CRGB::Red;
        if (gCylonPos > 0)
        {
            leds[gCylonPos - 1] += CRGB(64, 0, 0);
        }
        if (gCylonPos < NUM_LEDS - 1)
        {
            leds[gCylonPos + 1] += CRGB(64, 0, 0);
        }
    }

    void lightning()
    {
        // mostly dark strip with random bright flashes
        fadeToBlackBy(leds, NUM_LEDS, 40);

        if (random8() < 20)
        {
            uint8_t start = random8(NUM_LEDS);
            uint8_t len = random8(3, NUM_LEDS / 2);

            for (uint8_t i = 0; i < len && (start + i) < NUM_LEDS; ++i)
            {
                leds[start + i] = CRGB::White;
            }
        }
    }

    void colorWaves()
    {
        // smooth palette-based color waves along the strip
        static CRGBPalette16 palette = RainbowColors_p;

        for (uint8_t i = 0; i < NUM_LEDS; ++i)
        {
            uint8_t index = sin8(i * 8 + gHue * 2);
            uint8_t bright = sin8(i * 16 + gHue * 3);
            leds[i] = ColorFromPalette(palette, index, bright, LINEARBLEND);
        }
    }

    void noisePerlin()
    {
        // simple 1D Perlin/noise-based color strip
        for (uint8_t i = 0; i < NUM_LEDS; ++i)
        {
            // inoise8 is from FastLED
            uint8_t n = inoise8(i * 30, 0, gHue * 4);
            leds[i] = CHSV(n, 255, 255);
        }
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

    // EVERY_N_SECONDS(10) 
    // { 
    //     nextPattern(); 
    // }
}

// Called from BLE RGB characteristic (3 bytes: R,G,B)
void APP_LED::setSolidColor(uint8_t r, uint8_t g, uint8_t b)
{
    gSolidColor = CRGB(r, g, b);
}

// Called from BLE Animation characteristic (1 byte: 0-12)
void APP_LED::setAnimation(uint8_t animId)
{
    if (animId >= NUM_PATTERNS)
    {
        animId = 0; // fallback to Solid Color
    }

    gCurrentPattern = animId;
}