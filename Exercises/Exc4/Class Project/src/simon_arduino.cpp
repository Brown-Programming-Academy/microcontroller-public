// =====================================================================
// Simon game - Arduino version (ESP32-S3)
// ---------------------------------------------------------------------
// Hardware:
//   3 LEDs  (R, G, B)  - each one: GPIO -> 220 ohm resistor -> LED -> GND
//   3 push buttons     - each one: GPIO -> button -> GND
//                        (internal pull-up, so the pin reads LOW when pressed)
//   1 passive buzzer   - GPIO -> buzzer -> GND
//
// Game flow (repeats forever):
//   1. 5 beeps + blue led blinks          -> "a new game starts"
//   2. 10 rounds: in round N the board shows a sequence of N random leds
//      (1s each), then the player must press the matching buttons in the
//      same order. Each round adds one more random led to the sequence.
//   3. Wrong button -> red led + buzzer for 3s (game over)
//      All 10 rounds passed -> 10 beeps + green led blinks (victory)
//   4. Wait 2s and start again.
// =====================================================================

// The Arduino env compiles this file. The bare-metal env also uses the Arduino
// framework (for boot only), so we exclude this file when SIMON_BAREMETAL is set.
#if defined(ARDUINO) && !defined(SIMON_BAREMETAL)

#include <Arduino.h>

// ---------- pin assignment ----------

#define BUZZER_PIN 1

// ---------- game settings ----------
#define ROUNDS 10          // rounds needed to win
#define NUM_COLORS 3       // number of colors (leds / buttons): R, G, B
#define BUZZ_FREQ_HZ 2000  // buzzer tone frequency

// Lookup tables: index 0 = R, 1 = G, 2 = B.
// LED i and button i belong to the same color, so a color is just an index
// and we can compare "which led was shown" with "which button was pressed".

// The sequence the player has to repeat (one color index per step).
// It grows by one entry every round.
static uint8_t sequence[ROUNDS];

// Turn a led on or off (leds are active high: HIGH = on, LOW = off).
static void setLed(int pin, bool on)
{
    // TODO
}

// Sound the buzzer for 'ms' milliseconds (blocking).
// A passive buzzer has no oscillator inside - a constant HIGH makes no sound,
// so tone() generates a square wave (PWM) at BUZZ_FREQ_HZ on the pin.
static void buzz(uint32_t ms)
{
    tone(BUZZER_PIN, BUZZ_FREQ_HZ);
    delay(ms);
    noTone(BUZZER_PIN);
}

// Beep 'times' times while blinking 'ledPin' in sync with the beeps
// (led on + beep for 200ms, then led off + silence for 200ms).
static void beepAndBlink(int ledPin, int times)
{
    for (int i = 0; i < times; i++)
    {
        setLed(ledPin, true);
        buzz(200);
        setLed(ledPin, false);
        delay(200);
    }
}

// TODO
// Blocks until a button is pressed and released; returns its index (0=R, 1=G, 2=B)
static int waitForButton()
{
    return 0;
}

// Runs once after reset: configure the pins.
void setup()
{
    pinMode(BUZZER_PIN, OUTPUT);
    // TODO

}

// Runs again and again - every call to loop() is one full game.
void loop()
{
    // ----- 1. start indication -----
    beepAndBlink(LED_B_PIN, 5);
    delay(500); // short pause so the start blinks are not mixed with round 1

    // ----- 2. the rounds -----
    bool won = true;
    for (int round = 1; round <= ROUNDS && won; round++)
    {
        // Add one new random color (0..NUM_COLORS-1) to the end of the sequence.
        // On ESP32, random() is backed by the hardware random number generator.
        sequence[round - 1] = random(NUM_COLORS);

        // Show the whole sequence so far: each led on for 1s.
        // The 250ms off-gap lets the player see two identical colors in a row.
        for (int k = 0; k < round; k++)
        {
            setLed(LEDS[sequence[k]], true);
            delay(1000);
            setLed(LEDS[sequence[k]], false);
            delay(250);
        }

        // Read the sequence back from the player, step by step.
        // The first wrong button ends the game immediately.
        for (int k = 0; k < round; k++)
        {
            if (waitForButton() != sequence[k])
            {
                won = false;
                break;
            }
        }
        delay(500); // pause between rounds
    }

    // ----- 3. result -----
    if (won)
        beepAndBlink(LED_G_PIN, 10); // victory: 10 beeps + green blinks
    else
    {
        // failure: red led + continuous buzz for 3s
        setLed(LED_R_PIN, true);
        buzz(3000);
        setLed(LED_R_PIN, false);
    }

    // ----- 4. wait before the next game -----
    delay(2000);
}

#endif // ARDUINO && !SIMON_BAREMETAL
