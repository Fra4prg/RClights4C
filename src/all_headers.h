/*------------------------------------------------------------------------------
 * all_headers.h
 * collect all headers
 *
 * Copyright (c) 2014 Frank Scholl - MegaProjects@t-online.de
 *
 * ATTINY45 @ 8 MHz
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *------------------------------------------------------------------------------
 */


#ifndef _ALL_HEADERS_H
#define _ALL_HEADERS_H


/*------------------------------------------------------------------------------
 * basic includes
 *------------------------------------------------------------------------------
 */
#ifndef F_CPU
#define F_CPU 8000000
#endif

// select the target for the program. Possible entries:
#define TARGET_ATMEL_EVALBOARD_POLLIN	1	// ATMEL evaluation board from Pollin
#define TARGET_PROJECT					2	// Original PCB

// select the target for the program. Possible entries:
#define _TARGET_PCB		TARGET_PROJECT



/*------------------------------------------------------------------------------
 * standard includes
 *------------------------------------------------------------------------------
 */
#include <avr/io.h>
//#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>     // defines _delay_ms() from avr-libc Version 1.2.0
#include <inttypes.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/eeprom.h>
#include <stdlib.h>
#include <stdint.h>  // e.g. for uint16_t
//#include <string.h>
#include "bool.h"


/*------------------------------------------------------------------------------
 * my includes
 *------------------------------------------------------------------------------
 */
#include "Setup.h"
#include "Timer_ATtiny-25-45-85.h"

#include "RClights4C.h"


#endif //ALL_HEADERS
