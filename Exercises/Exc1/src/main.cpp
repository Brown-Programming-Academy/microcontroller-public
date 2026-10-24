#include <Arduino.h>

#define BUZZER_PIN 1
#define RGB_PIN 38 // onboard RGB LED in Wokwi (DevKitC-1 v1.1)

void setup() 
{
    pinMode(BUZZER_PIN, OUTPUT);
}

void loop() 
{
    tone(BUZZER_PIN, 1000); // passive buzzer needs a square wave, not a steady HIGH
    neopixelWrite(RGB_PIN, 0,RGB_BRIGHTNESS, 0); // pure red
    delay(1000);
    noTone(BUZZER_PIN);
    neopixelWrite(RGB_PIN, 0, 0, 0); // off
    delay(1000);
}
