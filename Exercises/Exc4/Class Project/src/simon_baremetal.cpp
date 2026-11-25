// =====================================================================
// Simon game - bare-metal version (ESP32-S3)
// ---------------------------------------------------------------------
// The hardware is accessed ONLY through raw memory-mapped registers -
// no Arduino / ESP-IDF driver function is called.
// The Arduino env is used just for the boot/startup code (clock setup,
// stack, calling setup()/loop()); everything after that is ours.
// Register addresses and bit fields are taken from the
// ESP32-S3 Technical Reference Manual (TRM).
//
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
// =====================================================================

// SIMON_BAREMETAL is defined only in the baremetal env (see platformio.ini).
#ifdef SIMON_BAREMETAL

#include <stdint.h>

// ---------- pin assignment ----------
#define LED_R_PIN 4
#define LED_G_PIN 5
#define LED_B_PIN 6
#define BTN_R_PIN 15
#define BTN_G_PIN 16
#define BTN_B_PIN 17
#define BUZZER_PIN 1

// ---------- game settings ----------
#define ROUNDS 10          // rounds needed to win
#define NUM_COLORS 3       // number of colors (leds / buttons): R, G, B
#define CPU_MHZ 240        // default CPU clock -> ccount ticks per microsecond
#define BUZZ_HALF_US 250   // half period of the tone: 250us high + 250us low = 2 kHz

// =====================================================================
// Registers (ESP32-S3 TRM)
// ---------------------------------------------------------------------
// Each peripheral register is a 32-bit word at a fixed address.
// 'volatile' tells the compiler that every read/write must really reach
// the hardware (it may not cache the value or optimize the access away).
// =====================================================================

// --- Hardware random number generator: each read returns a new 32-bit value ---
#define RNG_DATA (int)"TODO:Implement"

// Lookup tables: index 0 = R, 1 = G, 2 = B.
// LED i and button i belong to the same color, so a color is just an index
// and we can compare "which led was shown" with "which button was pressed".
static const int LEDS[NUM_COLORS] = {LED_R_PIN, LED_G_PIN, LED_B_PIN};
static const int BTNS[NUM_COLORS] = {BTN_R_PIN, BTN_G_PIN, BTN_B_PIN};

// The sequence the player has to repeat (one color index per step).
// It grows by one entry every round.
static uint8_t sequence[ROUNDS];

// =====================================================================
// Timing - using the CPU cycle counter
// ---------------------------------------------------------------------
// The Xtensa core has a special register CCOUNT that increments on every
// CPU clock cycle (240 MHz -> 240 ticks per microsecond). It is read with
// the 'rsr' (read special register) assembly instruction.
// =====================================================================
static inline uint32_t cycles()
{
    uint32_t c;
    asm volatile("rsr %0, ccount" : "=a"(c));
    return c;
}

// Busy-wait for 'us' microseconds.
// 'cycles() - start' is computed in unsigned 32-bit math, so it stays correct
// even when CCOUNT wraps around (every ~17.9s at 240 MHz).
static void delayUs(uint32_t us)
{
    uint32_t start = cycles();
    uint32_t ticks = us * CPU_MHZ;
    while (cycles() - start < ticks)
        ;
}

// Milliseconds are done as a loop of 1ms waits, to keep us * CPU_MHZ
// far away from 32-bit overflow.
static void delayMs(uint32_t ms)
{
    while (ms--)
        delayUs(1000);
}

// =====================================================================
// GPIO
// =====================================================================

// Configure a pin as a digital output, starting LOW.
static void pinAsOutput(int pin)
{
    // 1. IO MUX: select the GPIO function for this pin (keep the other bits)
    // 2. GPIO matrix: the pin is driven by its GPIO_OUT bit
    // 3. start LOW, then turn the output driver on
}

// Configure a pin as a digital input with the internal pull-up enabled.
static void pinAsInputPullup(int pin)
{
    // IO MUX: GPIO function + input buffer on + pull-up on (and pull-down off)
    // make sure the output driver is off, so the pin only listens
}

static void pinHigh(int pin) 
{
     
}

static void pinLow(int pin) 
{

}

// Buttons are wired to GND with a pull-up resistor:
// released -> bit is 1, pressed -> pin is shorted to GND -> bit is 0.
static bool isPressed(int pin) 
{ 
    return false;    
} // active low

// =====================================================================
// Game helpers
// =====================================================================

// Sound the buzzer for 'ms' milliseconds (blocking).
// A passive buzzer has no oscillator inside - a constant level makes no sound.
// We "bit-bang" a 2 kHz square wave: toggle the pin every 250us.
// Number of full periods = total time / period length (500us).
static void buzz(uint32_t ms)
{
    for (uint32_t i = 0; i < ms * 1000 / (2 * BUZZ_HALF_US); i++)
    {
        pinHigh(BUZZER_PIN);
        delayUs(BUZZ_HALF_US);
        pinLow(BUZZER_PIN);
        delayUs(BUZZ_HALF_US);
    }
}

// Beep 'times' times while blinking 'ledPin' in sync with the beeps
// (led on + beep for 200ms, then led off + silence for 200ms).
static void beepAndBlink(int ledPin, int times)
{
    for (int i = 0; i < times; i++)
    {
        pinHigh(ledPin);
        buzz(200);
        pinLow(ledPin);
        delayMs(200);
    }
}

// Blocks until a button is pressed and released; returns its index (0=R, 1=G, 2=B)
static int waitForButton()
{
    while (true) // poll the buttons until one of them is pressed
    {
        for (int i = 0; i < NUM_COLORS; i++)
        {
            if (!isPressed(BTNS[i]))
                continue;

            // Debounce: a mechanical button "bounces" between open/closed for
            // a few ms when pressed. Wait a bit and check again, so a short
            // glitch is not counted as a press.
            delayMs(20);
            if (!isPressed(BTNS[i]))
                continue;

            // Light the matching led while the button is held, as feedback
            // to the player that the press was registered.
            pinHigh(LEDS[i]);

            // Wait for release, otherwise one long press would be read as
            // several presses by the next calls.
            while (isPressed(BTNS[i]))
                ;
            pinLow(LEDS[i]);

            delayMs(20); // debounce the release as well
            return i;
        }
    }
}

// Runs once after reset: configure the pins.
void setup()
{
    for (int i = 0; i < NUM_COLORS; i++)
    {
        pinAsOutput(LEDS[i]);
        pinAsInputPullup(BTNS[i]);
    }
    pinAsOutput(BUZZER_PIN);
}

// Runs again and again - every call to loop() is one full game.
void loop()
{
    // ----- 1. start indication -----
    beepAndBlink(LED_B_PIN, 5);
    delayMs(500); // short pause so the start blinks are not mixed with round 1

    // ----- 2. the rounds -----
    bool won = true;
    for (int round = 1; round <= ROUNDS && won; round++)
    {
        // Add one new random color (0..NUM_COLORS-1) to the end of the sequence.
        sequence[round - 1] = RNG_DATA % NUM_COLORS;

        // Show the whole sequence so far: each led on for 1s.
        // The 250ms off-gap lets the player see two identical colors in a row.
        for (int k = 0; k < round; k++)
        {
            pinHigh(LEDS[sequence[k]]);
            delayMs(1000);
            pinLow(LEDS[sequence[k]]);
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
        pinHigh(LED_R_PIN);
        buzz(3000);
        pinLow(LED_R_PIN);
    }

    // ----- 4. wait before the next game -----
    delayMs(2000);
}

#endif // SIMON_BAREMETAL
