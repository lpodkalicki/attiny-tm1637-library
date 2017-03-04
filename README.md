# TM1637 Library
This is tinyAVR (ATtiny13, ATtiny25, ATtiny45, ATtiny85) library for 7-segment display modules based on TM1637 chip. The TM1637 chip also has keyboard input capability, but it's not implemented in this library.

Modules based on TM1637 provide two signal connections (CLK and DIO) and two power connections (VCC and GND). Signal pins can be connected to any pair of digital pins of the AVR chip. Signal pins configuration is defined at the top of library header file, where it can be modifed.

## Key Features
This lightweight library has the following features:
* display digits
* display raw segments
* brightness control
* display on/off
* software I2C

## Example Code
This example code demonstrates basic usage of the library:

```c
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include "tm1637.h"

int
main(void)
{
        uint8_t i = 0;

        /* setup */
        TM1637_init();
        TM1637_set_brightness(7);

        /* loop */
        while (1) {
                TM1637_display_digit(TM1637_SET_ADR_00H, i % 0x10);
                TM1637_display_digit(TM1637_SET_ADR_01H, (i + 1) % 0x10);
                TM1637_display_digit(TM1637_SET_ADR_02H, (i + 2) % 0x10);
                TM1637_display_digit(TM1637_SET_ADR_03H, (i + 3) % 0x10);
                _delay_ms(300);
                i++;
        }
}
```

## API Documentation (tm1637.h)

```c
/**
 * Initialize TM1637 display driver.
 * Clock pin (TM1637_CLK_PIN) and data pin (TM1637_DIO_PIN)
 * are defined at the top of this file.
 */
void TM1637_init(void);

/**
 * Set display brightness.
 * Min brightness: 0
 * Max brightness: 7
 */
void TM1637_set_brightness(const uint8_t brightness);

/**
 * Display raw segments at positions (0x00..0x03)
 *
 *      bits:                 hex:
 *        -- 0 --               -- 01 --
 *       |       |             |        |
 *       5       1            20        02
 *       |       |             |        |
 *        -- 6 --               -- 40 --
 *       |       |             |        |
 *       4       2            10        04
 *       |       |             |        |
 *        -- 3 --               -- 08 --
 *
 * Example segment configurations:
 * - for character 'H', segments=0b01110110
 * - for character '-', segments=0b01000000
 * - etc.
 */
void TM1637_display_segments(const uint8_t addr, const uint8_t segments);

/**
 * Display digits ('0'..'9') at positions (0x00..0x03)
 */
void TM1637_display_digit(const uint8_t addr, const uint8_t digit);

/**
 * Turn display on/off.
 */
void TM1637_enable(const bool value);

```
