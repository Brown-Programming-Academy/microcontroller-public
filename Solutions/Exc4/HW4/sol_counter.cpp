// =====================================================================
// HW 4 - Binary button counter (ESP32-S3) - SOLUTION
// ---------------------------------------------------------------------
// Hardware:
//   3 LEDs     - each one: GPIO -> 220 ohm resistor -> LED -> GND
//                LED0 = bit 0 (LSB), LED2 = bit 2 (MSB)
//   2 buttons  - each one: GPIO -> button -> GND
//                (internal pull-up, so the pin reads 0 when pressed)
//
// Behavior:
//   A counter 0..7 is shown in binary on the 3 LEDs.
//   UP   button -> count + 1 (after 7 wraps to 0)
//   DOWN button -> count - 1 (before 0 wraps to 7)
//   Every change is printed:  "UP   -> Count: 5 (101)"
//
// One file, two frameworks:
//   - the hardware layer (pins, read/write, delay, print) is written twice,
//     once inside #ifdef ARDUINO and once in the #else (ESP-IDF) part
//   - the logic (showBinary, waitForButton, the counter) is shared
// =====================================================================

// ---------- pin assignment (all P2 pins - datasheet p.26) ----------
#define LED0_PIN 4      // bit 0 (LSB)
#define LED1_PIN 5      // bit 1
#define LED2_PIN 6      // bit 2 (MSB)
#define BTN_UP_PIN 15   // count + 1
#define BTN_DOWN_PIN 16 // count - 1

// ---------- settings ----------
#define NUM_LEDS 3
#define NUM_BTNS 2
#define COUNT_RANGE (1 << NUM_LEDS) // 3 bits -> 8 values (0..7)
#define DEBOUNCE_MS 20              // how long a bounce may last
#define POLL_MS 10                  // sleep between button scans

// Button indexes, also the return values of waitForButton()
#define BTN_UP 0
#define BTN_DOWN 1

// Lookup tables. LEDS[i] shows bit i of the counter.
static const int LEDS[NUM_LEDS] = {LED0_PIN, LED1_PIN, LED2_PIN};
static const int BTNS[NUM_BTNS] = {BTN_UP_PIN, BTN_DOWN_PIN};

// =====================================================================
// Hardware layer - the only part that differs between the frameworks
// =====================================================================
#ifdef ARDUINO

#include <Arduino.h>

#define PRINT(...) Serial.printf(__VA_ARGS__)

// On Arduino-ESP32 delay() calls vTaskDelay(), so the task really sleeps.
static void delayMs(uint32_t ms)
{
    delay(ms);
}

// LEDs are active high: HIGH = on, LOW = off.
static void setLed(int pin, bool on)
{
    digitalWrite(pin, on ? HIGH : LOW);
}

// Buttons are wired to GND with a pull-up: pressed = LOW.
static bool isPressed(int pin)
{
    return digitalRead(pin) == LOW;
}

static void initHardware()
{
    Serial.begin(115200);
    for (int i = 0; i < NUM_LEDS; i++)
        pinMode(LEDS[i], OUTPUT);
    for (int i = 0; i < NUM_BTNS; i++)
        pinMode(BTNS[i], INPUT_PULLUP); // enable the internal pull-up resistor
}

#else // ---------------------- ESP-IDF ----------------------

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define PRINT(...) printf(__VA_ARGS__)

// vTaskDelay() works in RTOS ticks; pdMS_TO_TICKS converts ms to ticks.
// The task sleeps, so the idle task runs and the task watchdog stays happy.
static void delayMs(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

// The pin tables hold plain ints; ESP-IDF wants gpio_num_t (an enum),
// and C++ does not convert int -> enum implicitly, so we cast.
static void setLed(int pin, bool on)
{
    gpio_set_level((gpio_num_t)pin, on);
}

static bool isPressed(int pin)
{
    return gpio_get_level((gpio_num_t)pin) == 0; // active low
}

static void initHardware()
{
    // "= {}" zeroes all fields - gpio_config_t is a plain C struct, so
    // without it the fields we don't set would hold stack garbage.
    gpio_config_t out = {};
    for (int i = 0; i < NUM_LEDS; i++)
        out.pin_bit_mask |= 1ULL << LEDS[i]; // 1ULL: the mask is 64 bits wide
    out.mode = GPIO_MODE_OUTPUT;
    gpio_config(&out);

    gpio_config_t in = {};
    for (int i = 0; i < NUM_BTNS; i++)
        in.pin_bit_mask |= 1ULL << BTNS[i];
    in.mode = GPIO_MODE_INPUT;
    in.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&in);
}

#endif // ARDUINO

// =====================================================================
// Shared logic - identical for both frameworks
// =====================================================================

// The current counter value (0..COUNT_RANGE-1)
static int count = 0;

// Step 1: LED i shows bit i of n.
// (n >> i) moves bit i to position 0, "& 1" keeps only that bit.
static void showBinary(int n)
{
    for (int i = 0; i < NUM_LEDS; i++)
        setLed(LEDS[i], (n >> i) & 1);
}

// Prints e.g. "UP   -> Count: 5 (101)" - bits from MSB to LSB.
static void printCount(const char *label, int n)
{
    PRINT("%-4s -> Count: %d (%d%d%d)\n", label, n, (n >> 2) & 1, (n >> 1) & 1, n & 1);
}

// Step 2: blocks until a button is pressed and released.
// Returns its index (BTN_UP / BTN_DOWN) - exactly once per press.
static int waitForButton()
{
    while (true)
    {
        for (int i = 0; i < NUM_BTNS; i++)
        {
            if (!isPressed(BTNS[i]))
                continue;

            // Press debounce: a bounce is a short glitch, a real press is
            // still there after DEBOUNCE_MS.
            delayMs(DEBOUNCE_MS);
            if (!isPressed(BTNS[i]))
                continue;

            // Wait for release, so a long hold counts only once.
            while (isPressed(BTNS[i]))
                delayMs(POLL_MS);

            // Release debounce: let the contacts settle, otherwise the next
            // call could read a release bounce as a new press.
            delayMs(DEBOUNCE_MS);
            return i;
        }
        // Sleep between scans instead of spinning - frees the CPU (and in
        // ESP-IDF prevents the task watchdog from firing).
        delayMs(POLL_MS);
    }
}

static void counterInit()
{
    initHardware();
    count = 0;
    showBinary(count); // all LEDs off
    PRINT("Count: %d (000)\n", count);
}

// Step 3: one button press -> update, show, print.
static void counterStep()
{
    if (waitForButton() == BTN_UP)
    {
        count = (count + 1) % COUNT_RANGE; // 7 + 1 -> 0
        showBinary(count);
        printCount("UP", count);
    }
    else
    {
        // + COUNT_RANGE before % so 0 - 1 gives 7 and not -1
        // (in C, -1 % 8 == -1).
        count = (count - 1 + COUNT_RANGE) % COUNT_RANGE;
        showBinary(count);
        printCount("DOWN", count);
    }
}

// Experiment (step 2): with the two delayMs(DEBOUNCE_MS) calls removed,
// 10 presses on real hardware usually advance the counter by more than 10,
// because some bounces are counted as extra presses. The exact number
// changes from run to run.

// =====================================================================
// Entry points
// =====================================================================
#ifdef ARDUINO

void setup()
{
    counterInit();
}

// loop() is called again and again - each call handles one press.
void loop()
{
    counterStep();
}

#else

// ESP-IDF entry point. extern "C" because the IDF startup code is C.
extern "C" void app_main()
{
    counterInit();
    while (true)
        counterStep();
}

#endif // ARDUINO
