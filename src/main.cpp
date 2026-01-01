#define MDEBUG = 1

#include "Knobreader.cpp"

#include "pwm_helper.c"

#include <Arduino.h>

#include <FastLED.h>
// #include "pio_encoder.h"

// How many leds in your strip?
#define NUM_LEDS 1

#define DATA_PIN p23

// Define the array of leds
CRGB leds[NUM_LEDS];

// PioEncoder encA(p3);


auto fA = 8;
auto fB = 8;
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

void resetAll()
{
  resetKnobA();
  resetKnobB();
}
uint sliceL;
uint channelL;
uint sliceA;
uint channelA;
uint sliceB;
uint channelB;

// todo: make a motor controller abstraction
void setup()
{
  Serial.begin(9600);
  delay(2000);
  Serial.write("Hello");
  Serial.println(" gagi");
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
  pinMode(p25, OUTPUT);

  gpio_set_function(PICO_DEFAULT_LED_PIN, GPIO_FUNC_PWM);

  gpio_set_function(19, GPIO_FUNC_PWM);
  gpio_set_function(20, GPIO_FUNC_PWM);
  sliceL = pwm_gpio_to_slice_num(PICO_DEFAULT_LED_PIN);
  channelL = pwm_gpio_to_channel(PICO_DEFAULT_LED_PIN);
  sliceA = pwm_gpio_to_slice_num(19);
  channelA = pwm_gpio_to_channel(19);
  sliceB = pwm_gpio_to_slice_num(20);
  channelB = pwm_gpio_to_channel(20);
  pwm_set_freq_duty(sliceL, channelL, 8, 2);
}

void loop()
{

  auto value = knobA->updateAndReturn();
  // turn the led on, then pause
  if (value != nullptr)
  {
    digitalWrite(p18, *value < 0);
    pwm_set_freq_duty(sliceA, channelA, 10, *value);

    leds[0] = (leds[0] & 0xFFFF00) | (abs(*value) & 0x0000FF);

    FastLED.show();
  }

  value = knobB->updateAndReturn();

  // turn the led on, then pause
  if (value != nullptr)
  {
    digitalWrite(p21, *value < 0);
    pwm_set_freq_duty(sliceB, channelB, 2000, *value);
    leds[0] = (leds[0] & 0xFF00FF) | (abs(*value) << 8);
    FastLED.show();
  }

  // Serial.print("4: ");
  // Serial.print(digitalRead(4));
  // Serial.print(" 5: ");
  // Serial.println(digitalRead(5));

  // Serial.print("num interrupts: ");
  // Serial.println(knobA->encoder->getState().interruptCount);
  // Serial.print("num trans same direction: ");
  // Serial.println(knobA->encoder->getState().successfulTransitions);
  // Serial.print("drection: ");
  // Serial.println(knobA->encoder->getState().direction);

  delay(250);
}

// put function declarations here:
