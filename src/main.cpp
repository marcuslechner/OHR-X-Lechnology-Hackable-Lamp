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
 #include <FastLED.h>
 #include <ESP32Servo.h>
 
 // ==============================
 // LED Strip Configuration
 // ==============================
 
 #define DATA_PIN        4
 #define LED_TYPE        WS2812
 #define COLOR_ORDER     GRB
 #define NUM_LEDS        24
 #define BRIGHTNESS      255
 #define FRAMES_PER_SECOND 120
 
 CRGB leds[NUM_LEDS];
 
 // ==============================
 // Servo Configuration
 // ==============================
 
 Servo myservo;                 // Servo control object
#define SERVO_PIN 18       // GPIO pin for servo signal (digital output)
 
 // ==============================
 // Potentiometer Configuration
 // ==============================
 
#define POT_PIN 34         // GPIO pin for potentiometer (analog input)
#define ADC_MAX 4096      // 12-bit ADC (range 0-4095), configurable 9–12 bits
 
 // ==============================
 // State Variables
 // ==============================
 
 int val = 0;                   // Current analog reading
 int previous_val = 0;          // Previous reading
 int steps_til_release = 0;     // Placeholder for control logic
 int servo_position = 0;        // Last commanded servo position
 

void rainbow(void);
void nextPattern(void);
void rainbowWithGlitter(void);
void confetti(void);
void sinelon(void);
void juggle(void);
void bpm(void);
void addGlitter(fract8 chanceOfGlitter);


void setup() 
{
  delay(3000); // 3 second delay for recovery

  	// Allow allocation of all timers
  Serial.begin(115200);
	ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);// Standard 50hz servo
  myservo.attach(SERVO_PIN, 500, 2400);   // attaches the servo on pin 18 to the servo object
                                         // using SG90 servo min/max of 500us and 2400us
                                         // for MG995 large servo, use 1000us and 2000us,
                                         // which are the defaults, so this line could be
                                         // "myservo.attach(servoPin);"
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])(); //type definition of an array of function pointers that take void param and return void
SimplePatternList gPatterns = { 
  rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm 
};

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
  
void loop()
{

  //APP_ANIMATION_PROCESS();
  //HDW_SERVO_PROCESS();
  // Call the current pattern function once, updating the 'leds' array
  // gPatterns[gCurrentPatternNumber]();

  // // send the 'leds' array out to the actual LED strip
  // FastLED.show();  
  // // insert a delay to keep the framerate modest
  // FastLED.delay(1000/FRAMES_PER_SECOND); 

  // // do some periodic updates
  // EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  // EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
      // GPIO pin for potentiometer (analog input)

  val = analogRead(POT_PIN);            // read the value of the potentiometer (value between 0 and 1023)
  val = map(val, 250, ADC_MAX-250, 60, 100 );     // scale it to use it with the servo (value between 0 and 180) 

  // servo_position = myservo.read();
  // Serial.println(servo_position);
  // Serial.println(previous_val);
  Serial.print("val ");
  Serial.println(val);

  myservo.write(val); 

  delay(100);
  // if(abs(val-previous_val) > 1)
  // {
  //     myservo.write(val);                  // set the servo position according to the scaled value
  //     previous_val = val;
  //     // Serial.println("sending");
  //     steps_til_release = 0;
      
  // }
  // else
  // {
  //   steps_til_release++;
  //   if(steps_til_release > 20)
  //   {
  //     myservo.release();
  //   }
  // }
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0])) //TODO: where does it get this?

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  uint8_t dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
