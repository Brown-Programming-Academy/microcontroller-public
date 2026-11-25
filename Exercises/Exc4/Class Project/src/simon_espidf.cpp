// =====================================================================
// Simon game - ESP-IDF version (ESP32-S3)
// ---------------------------------------------------------------------
// Hardware:
//   3 LEDs  (R, G, B)  - each one: GPIO -> 220 ohm resistor -> LED -> GND
//   3 push buttons     - each one: GPIO -> button -> GND
//                        (internal pull-up, so the pin reads 0 when pressed)
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
//
// ESP-IDF APIs used:
//   driver/gpio.h  - configure pins, read buttons, drive leds
//   driver/ledc.h  - the LEDC (PWM) peripheral generates the buzzer tone
//   esp_random.h   - hardware random number generator
//   FreeRTOS       - vTaskDelay() for delays (lets other tasks run meanwhile)
// =====================================================================

// ARDUINO is defined only by the Arduino framework, so this file compiles
// only in the espidf env.

#ifndef ARDUINO

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_random.h"

// ---------- pin assignment ----------

#define BUZZER_PIN GPIO_NUM_1

// ---------- game settings ----------
#define ROUNDS 10                   // rounds needed to win
#define NUM_COLORS 3                // number of colors (leds / buttons): R, G, B
#define BUZZ_FREQ_HZ 2000           // buzzer tone frequency
#define BUZZ_CHANNEL LEDC_CHANNEL_0 // LEDC channel that drives the buzzer

// The sequence the player has to repeat (one color index per step).
// It grows by one entry every round.
static uint8_t sequence[ROUNDS];

// vTaskDelay() works in RTOS ticks, pdMS_TO_TICKS converts milliseconds to ticks.
// Unlike a busy-wait, the task sleeps and the CPU is free for other tasks
// (and the idle task keeps feeding the watchdog).
static void delayMs(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

// TODO
// LEDs are wired GPIO -> resistor -> LED -> GND, so 1 = on, 0 = off.
static void setLed(gpio_num_t pin, bool on)
{
    
}

// TODO
static void initHardware()
{
    // --- LEDs: plain push-pull outputs ---
    // gpio_config() can set up several pins at once, given as a bit mask.
    
    // --- Buttons: inputs with the internal pull-up enabled ---

    // --- Buzzer ---
    // A passive buzzer has no oscillator inside - a constant level makes no
    // sound. We need a square wave, which the LEDC (PWM) peripheral generates
    // in hardware: a timer sets the frequency, a channel connects it to a pin.

    // Timer: 2 kHz, 10-bit duty resolution (duty values 0..1023)
    ledc_timer_config_t timer = {};
    timer.speed_mode = LEDC_LOW_SPEED_MODE; // the S3 only has low-speed mode
    timer.duty_resolution = LEDC_TIMER_10_BIT;
    timer.timer_num = LEDC_TIMER_0;
    timer.freq_hz = BUZZ_FREQ_HZ;
    timer.clk_cfg = LEDC_AUTO_CLK;
    ledc_timer_config(&timer);

    // Channel: routes timer 0 to the buzzer pin. Duty 0 = silent at start.
    ledc_channel_config_t channel = {};
    channel.gpio_num = BUZZER_PIN;
    channel.speed_mode = LEDC_LOW_SPEED_MODE;
    channel.channel = BUZZ_CHANNEL;
    channel.timer_sel = LEDC_TIMER_0;
    channel.duty = 0;
    ledc_channel_config(&channel);
}

// Sound the buzzer for 'ms' milliseconds (blocking).
// Duty 512 of 1024 = 50% -> a symmetric square wave (loudest tone);
// duty 0 -> output stays low -> silence.
// ledc_set_duty() only stages the new value, ledc_update_duty() applies it.
static void buzz(uint32_t ms)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BUZZ_CHANNEL, 512);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BUZZ_CHANNEL);
    delayMs(ms);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BUZZ_CHANNEL, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BUZZ_CHANNEL);
}

// Beep 'times' times while blinking 'ledPin' in sync with the beeps
// (led on + beep for 200ms, then led off + silence for 200ms).
static void beepAndBlink(gpio_num_t ledPin, int times)
{
    for (int i = 0; i < times; i++)
    {
        setLed(ledPin, true);
        buzz(200);
        setLed(ledPin, false);
        delayMs(200);
    }
}

// TODO
// Blocks until a button is pressed and released; returns its index (0=R, 1=G, 2=B)
static int waitForButton()
{
    return 0;
}

// ESP-IDF entry point (instead of main). It runs inside a FreeRTOS task.
// extern "C" is needed because the IDF startup code (C) calls it by its C name.
extern "C" void app_main()
{
    initHardware();

    while (true) // every iteration is one full game
    {
        // ----- 1. start indication -----
        beepAndBlink(LED_B_PIN, 5);
        delayMs(500); // short pause so the start blinks are not mixed with round 1

        // ----- 2. the rounds -----
        bool won = true;
        for (int round = 1; round <= ROUNDS && won; round++)
        {
            // Add one new random color (0..NUM_COLORS-1) to the end of the sequence,
            // taken from the hardware random number generator.
            sequence[round - 1] = esp_random() % NUM_COLORS;

            // Show the whole sequence so far: each led on for 1s.
            // The 250ms off-gap lets the player see two identical colors in a row.
            for (int k = 0; k < round; k++)
            {
                setLed(LEDS[sequence[k]], true);
                delayMs(1000);
                setLed(LEDS[sequence[k]], false);
                delayMs(250);
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
            delayMs(500); // pause between rounds
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
        delayMs(2000);
    }
}

#endif // !ARDUINO
