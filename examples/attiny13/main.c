/**
 * Copyright (c) 2017, Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 *
 * This is ATtiny13 "Running Digits" example using attiny-tm1637-library,
 * https://github.com/lpodkalicki/attiny-tm1637-library .
 *
 */

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

