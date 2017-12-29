/* Copyright (c) 2017-2018, Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * Some examples of how to use attiny-tm1637-library,
 * https://github.com/lpodkalicki/attiny-tm1637-library.
 *
 */

#define F_CPU 1000000UL

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>

#include "tm1637.h"

void displayInAllSegments(uint8_t segment);
void displayTemperature(float t, const char unit);

int
main(void) {
    uint8_t j = 0;

    /* setup */
    TM1637_init(PB0, PB1);

    /* loop */
    while (1) {
        // This is ATtiny13 "Running Digits" example using attiny-tm1637-library,
        for (uint8_t i = 0; i < 200; i++) {
            TM1637_display_digit(TM1637_SET_ADR_00H, i % 0x10);
            TM1637_display_digit(TM1637_SET_ADR_01H, (i + 1) % 0x10);
            TM1637_display_digit(TM1637_SET_ADR_02H, (i + 2) % 0x10);
            TM1637_display_digit(TM1637_SET_ADR_03H, (i + 3) % 0x10);
            TM1637_display_colon(true);
            _delay_ms(200);
            TM1637_display_colon(false);
            _delay_ms(200);
        }
        TM1637_clear();
        displayInAllSegments(0x1);
        _delay_ms(200);
        displayInAllSegments(0x40);
        _delay_ms(200);
        displayInAllSegments(0x08);
        _delay_ms(200);
        TM1637_clear();
        _delay_ms(200);
        // digit numbering from left to right: 0 1 2 3
        TM1637_display_digit(2, j++ % 10); // digit second from right
        _delay_ms(2000);
        TM1637_display_digit(0, j++ % 10); // left most digit
        _delay_ms(2000);
        TM1637_display_digit(3, j++ % 10); // right most digit
        _delay_ms(2000);
        TM1637_display_digit(1, j++ % 10); // digit second from left
        _delay_ms(2000);
        j %= 7; // keep j in range 0 to 6
        displayInAllSegments(0x08);
        _delay_ms(200);
        displayInAllSegments(0x40);
        _delay_ms(200);
        displayInAllSegments(0x1);
        _delay_ms(1000);
        TM1637_display_text("Error-Error-Error");
        _delay_ms(1000);
        TM1637_clear();
        _delay_ms(500);
        // switch on colon until disabled again
        TM1637_display_colon(1);
        TM1637_display_text("203C");
        _delay_ms(1000);
        TM1637_display_text("451H");
        _delay_ms(2000);
        displayTemperature(-9.26, 'C');
        _delay_ms(3000);
        displayTemperature(29.26, 'F');
        _delay_ms(3000);
        displayTemperature(-0.26, 'C');
        _delay_ms(3000);
        displayTemperature(100.26, 'C');
        _delay_ms(3000);
        // blinking colon
        TM1637_display_colon(0);
        TM1637_display_text("291H");
        _delay_ms(1000);
        TM1637_display_colon(1);
        _delay_ms(1000);
        TM1637_display_colon(0);
        _delay_ms(1000);
        TM1637_display_colon(1);
        _delay_ms(1000);
        // colon is still switched on
        TM1637_display_text("HELLO");
        _delay_ms(3000);
        // clear display including colon
        TM1637_clear();
        _delay_ms(3000);
        TM1637_display_text("0123456789");
        _delay_ms(3000);
    }
}

void displayInAllSegments(uint8_t segment) {
    TM1637_display_segments(0, segment);
    TM1637_display_segments(1, segment);
    TM1637_display_segments(2, segment);
    TM1637_display_segments(3, segment);
}

void displayTemperature(float t, const char unit) {
    // range -9.9 degrees Celsius up to 99.9 degress Celsius or Fahrenheit
    if (t >= -9.9 && t <= 99.9) {
        TM1637_display_colon(1);

        uint8_t pos = 0;

        // position 0 (outer left)
        if (t < 0) {
            TM1637_display_segments(pos++, 0x40);
            t = -t;
        }
        int d = 0;
        if (pos == 0 && t > 10) {
            d = (int) t / 10;
            TM1637_display_digit(pos++, d);
            t = t - d * 10.0;
        } else if (pos == 0) {
            TM1637_display_segments(pos++, 0x0);
        }
        // position 1
        d = (int) t;
        TM1637_display_digit(pos++, d);
        d = (t - d) * 10.0 + 0.5;
        // position 2
        TM1637_display_digit(pos++, d);
        // position 3 (outer right)
        if (unit == 'C') {
            TM1637_display_segments(pos, 0x39);
        } else if (unit == 'F') {
            TM1637_display_segments(pos, 0x71);
        }
    } else {
        TM1637_clear();
        char *msg = (char *) calloc(30, sizeof (char));
        sprintf(msg, "Error_%d_%3d_Error", (int) t, (int) ((t - (int) t)*100));
        TM1637_display_text(msg);
        free(msg);
    }
}
