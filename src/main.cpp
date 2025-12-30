#define MDEBUG = 1
#include "Knobreader.h"

#include <Arduino.h>

#include <FastLED.h>
#include "RP2040_PWM.h"
// #include "pio_encoder.h"

// How many leds in your strip?
#define NUM_LEDS 1

#define DATA_PIN p23

// Define the array of leds
CRGB leds[NUM_LEDS];

// PioEncoder encA(p3);

RP2040_PWM *pwmA;
RP2040_PWM *pwmB;

auto fA = 20.0f;
auto fB = 200.0f;
KnobReader *knobA;
KnobReader *knobB;
void resetKnobA()
{
  knobA->setValue(0);
}
void resetKnobB()
{
  knobB->setValue(0);
}

void resetAll() {
  resetKnobA();
  resetKnobB();
}

// todo: make a motor controller abstraction
void setup()
{
  Serial.begin(9600);
  delay(2000);
  Serial.write("Hello");
  Serial.println(" gagi");
  pwmA = new RP2040_PWM(p19, fA, 0);
  pwmB = new RP2040_PWM(p20, fB, 0);
  pwmA->enablePWM();
  pwmB->enablePWM();
  // Uncomment/edit one of the following lines for your leds arrangement.
  // ## Clockless types ##
  FastLED.addLeds<WS2812, DATA_PIN>(leds, NUM_LEDS); // GRB ordering is assumed
  // RP2040_PWM().setPWM
  // encA.begin();
  pinMode(p3, INPUT);
  pinMode(p4, INPUT);
  pinMode(p5, INPUT);
  pinMode(p11, INPUT);
  pinMode(p12, INPUT);
  pinMode(p13, INPUT);
  pinMode(p24, INPUT);

  attachInterrupt(digitalPinToInterrupt(p3), resetKnobA, RISING);
  attachInterrupt(digitalPinToInterrupt(p11), resetKnobB, RISING);
  attachInterrupt(digitalPinToInterrupt(p24), resetAll, RISING);

  knobA = new KnobReader(p4, p5, -100, 100);
  knobB = new KnobReader(p12, p13, -100, 100);

  pinMode(p21, OUTPUT);
  pinMode(p20, OUTPUT);
  pinMode(p19, OUTPUT);
  pinMode(p18, OUTPUT);
}

void loop()
{

  auto value = knobA->updateAndReturn();
  // turn the led on, then pause
  if (value != nullptr)
  {
    digitalWrite(p18, *value < 0);
    pwmA->setPWM(p19, fA, abs(*value));
    leds[0] = (leds[0] & 0xFFFF00) | (abs(*value) & 0x0000FF);

    FastLED.show();
  }

  value = knobB->updateAndReturn();
  // turn the led on, then pause
  if (value != nullptr)
  {
    digitalWrite(p21, *value < 0);
    pwmB->setPWM(p20, fB, abs(*value));
    leds[0] = (leds[0] & 0xFF00FF) | (abs(*value) << 8);
    FastLED.show();
  }

  delay(50);
}

// put function declarations here:
