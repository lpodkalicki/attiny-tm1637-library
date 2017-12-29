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
 * 
 * This is ATtiny13/25/45/85 library for 4-Digit LED Display based on TM1637 chip.
 *
 * Features:
 * - display digits
 * - display colon
 * - display raw segments
 * - display on/off
 * - brightness control
 *
 * References:
 * - library: https://github.com/lpodkalicki/attiny-tm1637-library
 * - documentation: https://github.com/lpodkalicki/attiny-tm1637-library/README.md
 * - TM1637 datasheet: https://github.com/lpodkalicki/attiny-tm1637-library/blob/master/docs/TM1637_V2.4_EN.pdf
 * - http://wiki.seeedstudio.com/images/c/c3/TM1637_datasheet.pdf
 */

#define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>
#include "tm1637.h"

#define TM1637_HIGH(pin)        (PORTB |= _BV(pin))
#define TM1637_LOW(pin)         (PORTB &= ~_BV(pin))
#define TM1637_OUTPUT(pin)      (DDRB |= _BV(pin))
#define TM1637_INPUT(pin)       (DDRB &= ~_BV(pin))
#define TM1637_READ(pin)        (((PINB & _BV(pin)) > 0) ? 1 : 0)

#define TM1637_FLAG_ENABLED     (1 << 0)
#define TM1637_FLAG_SHOWCOLON   (1 << 1)

#define ASCII_0                 48
#define ASCII_9                 57

static void TM1637_configure(void);
static void TM1637_cmd(uint8_t value);
static void TM1637_start(void);
static void TM1637_stop(void);
static uint8_t TM1637_write_byte(uint8_t value);
static uint8_t convertChar(char c);

static const uint8_t _digit2segments[] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F // 9
};

static uint8_t displayedSegments[4] = {0, 0, 0, 0};

static uint8_t _brightness = TM1637_DEFAULT_BRIGHTNESS;
static uint8_t _flags = 0x00;

static uint8_t dio = TM1637_DIO_PIN;
static uint8_t clk = TM1637_CLK_PIN;

void
TM1637_init(const uint8_t dioPin, const uint8_t clkPin) {
    dio = dioPin;
    clk = clkPin;

    displayedSegments[0] = displayedSegments[1] = displayedSegments[2] = displayedSegments[3] = 0;

    // make both pins output pins thereby implicitly set their level to high
    DDRB |= (_BV(dio) | _BV(clk));
    _flags |= TM1637_FLAG_ENABLED;
    TM1637_clear();
}

void TM1637_display_text(const char* text) {
    uint8_t l = 0;
    while (text[l++] != '\0');

    uint8_t displayColon = _flags & TM1637_FLAG_SHOWCOLON ? 0x80 : 0x00;

    uint8_t segment[4] = {0, 0, 0, 0};
    for (uint8_t i = 0; i < l; i++) {
        for (uint8_t j = 0; j < 4; j++) TM1637_display_segments(j, displayColon | segment[j]);
        segment[0] = segment[1];
        segment[1] = segment[2];
        segment[2] = segment[3];
        segment[3] = convertChar(text[i]);
        _delay_ms(100);
    }
}

static uint8_t convertChar(char c) {
    uint8_t val = 0;

    if ((ASCII_0 <= c) && (c <= ASCII_9)) {
        return _digit2segments[c - ASCII_0];
    } else if ('-' == c) {
        return 0x40;
    } else if ('_' == c) {
        return 0x08;
    } else if ('A' == c) {
        return 0x77;
    } else if ('B' == c) {
        return _digit2segments[8];
    } else if ('C' == c) {
        return 0x39;
    } else if ('D' == c) {
        return _digit2segments[0];
    } else if ('E' == c) {
        return val | 0x79;
    } else if ('F' == c) {
        return val | 0x71;
    } else if ('G' == c) {
        return _digit2segments[6];
    } else if ('H' == c) {
        return 0x76;
    } else if ('I' == c) {
        return _digit2segments[1];
    } else if ('J' == c) {
        return 0x0F;
    } else if ('L' == c) {
        return 0x38;
    } else if ('O' == c) {
        return _digit2segments[0];
    } else if ('P' == c) {
        return 0x73;
    } else if ('R' == c) {
        return _digit2segments[0];
    } else if ('S' == c) {
        return 0x6C;
    } else if ('U' == c) {
        return 0x3D;
    } else if ('d' == c) {
        return 0x5E;
    } else if ('h' == c) {
        return 0x74;
    } else if ('o' == c) {
        return 0x5C;
    } else if ('r' == c) {
        return 0x50;
    } else if ('u' == c) {
        return 0x94;
    }

    return val;
}

void
TM1637_display_digit(const uint8_t addr, const uint8_t digit) {
    TM1637_display_segments(addr % 4, digit < 10 ? _digit2segments[digit] : 0x00);
}

void
TM1637_display_segments(const uint8_t addr, const uint8_t segments) {
    if (displayedSegments[addr] != segments) {
        displayedSegments[addr] = segments;
        TM1637_cmd(TM1637_CMD_SET_DATA | TM1637_SET_DATA_F_ADDR);
        TM1637_start();
        TM1637_write_byte(TM1637_CMD_SET_ADDR | addr);
        TM1637_write_byte(_flags & TM1637_FLAG_SHOWCOLON ? 0x80 | segments : segments);
        TM1637_stop();
    }
    TM1637_configure();
}

void
TM1637_display_colon(bool value) {
    if (value) {
        _flags |= TM1637_FLAG_SHOWCOLON;
        // explicitly switch on colon to overcome filter in TM1637_display_segments
        TM1637_display_segments(1, 0x80 | displayedSegments[1]);
    } else {
        _flags &= ~TM1637_FLAG_SHOWCOLON;
        // explicitly switch off colon to overcome filter in TM1637_display_segments
        TM1637_display_segments(1, ~0x80 & displayedSegments[1]);
    }
}

void
TM1637_clear(void) {
    TM1637_display_colon(false);
    TM1637_display_segments(TM1637_SET_ADR_00H, 0x00); // also sets displayedSegments[0] to 0
    TM1637_display_segments(TM1637_SET_ADR_01H, 0x00); // also sets displayedSegments[1] to 0
    TM1637_display_segments(TM1637_SET_ADR_02H, 0x00); // also sets displayedSegments[2] to 0
    TM1637_display_segments(TM1637_SET_ADR_03H, 0x00); // also sets displayedSegments[3] to 0
}

void
TM1637_set_brightness(const uint8_t brightness) {
    _brightness = brightness & 0x07;
    TM1637_configure();
}

void TM1637_display_on(void) {
    TM1637_enable(true);
}

void TM1637_display_off(void) {
    TM1637_enable(false);
}

void
TM1637_enable(bool value) {
    if (value) {
        _flags |= TM1637_FLAG_ENABLED;
    } else {
        _flags &= ~TM1637_FLAG_ENABLED;
    }
    TM1637_configure();
}

void
TM1637_configure(void) {
    uint8_t cmd = TM1637_CMD_SET_DISPLAY;
    cmd |= _brightness;
    if (_flags & TM1637_FLAG_ENABLED) {
        cmd |= TM1637_SET_DISPLAY_ON;
    }
    TM1637_cmd(cmd);
}

void
TM1637_cmd(uint8_t value) {
    TM1637_start();
    TM1637_write_byte(value);
    TM1637_stop();
}

void
TM1637_start(void) {
    /*
     * When CLK is high, and DIO goes from high to low, input begins
     */
    PORTB |= _BV(clk) | _BV(dio);
    _delay_us(TM1637_DELAY_US);
    TM1637_LOW(dio);
}

void
TM1637_stop(void) {
    /*
     * When CLK is high, and DIO goes from low to high, input ends
     */
    TM1637_LOW(clk);
    _delay_us(TM1637_DELAY_US);
    TM1637_LOW(dio);
    _delay_us(TM1637_DELAY_US);
    TM1637_HIGH(clk);
    _delay_us(TM1637_DELAY_US);
    TM1637_HIGH(dio);
}

uint8_t
TM1637_write_byte(uint8_t value) {
    /*
     *Send each bit of data
     */
    for (uint8_t i = 0; i < 8; i++) {
        //transfer data when clock is low, from low bit to high bit
        TM1637_LOW(clk);
        if ((value >> i) & 0x01) {
            TM1637_HIGH(dio); // data bit equals 1
        } else {
            TM1637_LOW(dio); // data bit equals 0
        }
        _delay_us(TM1637_DELAY_US);
        TM1637_HIGH(clk);
        _delay_us(TM1637_DELAY_US);
    }
    /*
     * End of 8th clock cycle is start of ACK from TM1637
     */
    TM1637_LOW(clk);
    TM1637_INPUT(dio);
    _delay_us(TM1637_DELAY_US);

    uint8_t ack = TM1637_READ(dio);

    _delay_us(TM1637_DELAY_US);

    TM1637_HIGH(clk);
    _delay_us(TM1637_DELAY_US);

    TM1637_LOW(clk);
    TM1637_OUTPUT(dio);

    return ack;
}
