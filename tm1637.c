/**
 * Copyright (c) 2017, Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * 
 * This is ATtiny13/25/45/85 library for 4-Digit LED Display based on TM1637 chip.
 *
 * Features:
 * - display digits
 * - display raw segments
 * - display on/off
 * - brightness control  
 *
 * References:
 * - library: https://github.com/lpodkalicki/attiny-tm1637-library
 * - documentation: https://github.com/lpodkalicki/attiny-tm1637-library/README.md
 * - TM1637 datasheet: https://github.com/lpodkalicki/attiny-tm1637-library/blob/master/docs/TM1637_V2.4_EN.pdf
 */

#include <avr/io.h>
#include <util/delay.h>
#include "tm1637.h"

#define TM1637_DIO_HIGH()		(PORTB |= _BV(TM1637_DIO_PIN))
#define TM1637_DIO_LOW()		(PORTB &= ~_BV(TM1637_DIO_PIN))
#define TM1637_DIO_OUTPUT()		(DDRB |= _BV(TM1637_DIO_PIN))
#define TM1637_DIO_INPUT()		(DDRB &= ~_BV(TM1637_DIO_PIN))
#define	TM1637_DIO_READ() 		(((PINB & _BV(TM1637_DIO_PIN)) > 0) ? 1 : 0)
#define TM1637_CLK_HIGH()		(PORTB |= _BV(TM1637_CLK_PIN))
#define TM1637_CLK_LOW()		(PORTB &= ~_BV(TM1637_CLK_PIN))

static void TM1637_configure(void);
static void TM1637_cmd(uint8_t value);
static void TM1637_start(void);
static void TM1637_stop(void);
static uint8_t TM1637_write_byte(uint8_t value);

static const uint8_t _digit2segments[] =
{
	0x3F, // 0
	0x06, // 1
	0x5B, // 2
	0x4F, // 3
	0x66, // 4
	0x6D, // 5
	0x7D, // 6
	0x07, // 7
	0x7F, // 8
	0x6F  // 9
};

static uint8_t _brightness = TM1637_DEFAULT_BRIGHTNESS;
static bool _enabled = TM1637_DEFAULT_ENABLED;

void
TM1637_init(void)
{

	DDRB |= (_BV(TM1637_DIO_PIN)|_BV(TM1637_CLK_PIN));
	PORTB &= ~(_BV(TM1637_DIO_PIN)|_BV(TM1637_CLK_PIN));
}

void
TM1637_set_brightness(const uint8_t brightness)
{

	_brightness = brightness & 0x07;
	TM1637_configure();
}

void
TM1637_enable(bool value)
{

	_enabled = value;
	TM1637_configure();
}

void
TM1637_display_digit(const uint8_t addr, const uint8_t digit)
{

	TM1637_display_segments(addr, digit < 10 ? _digit2segments[digit] : 0x00);
}

void
TM1637_display_segments(const uint8_t addr, const uint8_t segments)
{

	TM1637_cmd(TM1637_CMD_SET_DATA | TM1637_SET_DATA_F_ADDR);
	TM1637_start();
	TM1637_write_byte(TM1637_CMD_SET_ADDR | addr);
	TM1637_write_byte(segments);
	TM1637_stop();	
	TM1637_configure();	
}

void
TM1637_clear(void)
{	
	TM1637_display_segments(TM1637_SET_ADR_00H, 0x00);
	TM1637_display_segments(TM1637_SET_ADR_01H, 0x00);
	TM1637_display_segments(TM1637_SET_ADR_02H, 0x00);
	TM1637_display_segments(TM1637_SET_ADR_03H, 0x00);
}

void
TM1637_configure(void)
{
	uint8_t cmd;
	cmd = TM1637_CMD_SET_DSIPLAY;
	cmd |= _brightness;
	cmd |= _enabled ? TM1637_SET_DISPLAY_ON : TM1637_SET_DISPLAY_OFF;
	TM1637_cmd(cmd);
}

void
TM1637_cmd(uint8_t value)
{

	TM1637_start();
	TM1637_write_byte(value);
	TM1637_stop();
}

void
TM1637_start(void)
{

	TM1637_DIO_HIGH();
	TM1637_CLK_HIGH();
	_delay_us(TM1637_DELAY_US);
	TM1637_DIO_LOW();
}

void
TM1637_stop(void)
{

	TM1637_CLK_LOW();
	_delay_us(TM1637_DELAY_US);

	TM1637_DIO_LOW();
	_delay_us(TM1637_DELAY_US);

	TM1637_CLK_HIGH();
	_delay_us(TM1637_DELAY_US);

	TM1637_DIO_HIGH();
}

uint8_t
TM1637_write_byte(uint8_t value)
{
	uint8_t i, ack;

	for (i = 0; i < 8; ++i, value >>= 1) {
		TM1637_CLK_LOW();
		_delay_us(TM1637_DELAY_US);

		if (value & 0x01) {
			TM1637_DIO_HIGH();
		} else {
			TM1637_DIO_LOW();
		}

		TM1637_CLK_HIGH();
		_delay_us(TM1637_DELAY_US);
	}

	TM1637_CLK_LOW();
	TM1637_DIO_INPUT();
	TM1637_DIO_HIGH();
	_delay_us(TM1637_DELAY_US);

	ack = TM1637_DIO_READ();
	if (ack) {
		TM1637_DIO_OUTPUT();
		TM1637_DIO_LOW();
	}
	_delay_us(TM1637_DELAY_US);

	TM1637_CLK_HIGH();
	_delay_us(TM1637_DELAY_US);

	TM1637_CLK_LOW();
	_delay_us(TM1637_DELAY_US);

	TM1637_DIO_OUTPUT();

	return ack;
}

