/**
 * Copyright (c) 2017-2018, Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
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
	uint8_t n, k = 0;

	/* setup */
	TM1637_init(1/*enable*/, 5/*brightness*/);

	/* loop */
	while (1) {
		for (n = 0; n < TM1637_POSITION_MAX; ++n) {
			TM1637_display_digit(n, (k + n) % 0x10);
		}
		TM1637_display_colon(1);
		_delay_ms(200);
		TM1637_display_colon(0);
		_delay_ms(200);
		k++;
	}
}
