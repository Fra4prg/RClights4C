/*------------------------------------------------------------------------------
 * Setup.h
 * generic initialisation routines for AeroLight_4C
 *
 * Copyright (c) 2012 Frank Scholl - MegaProjects@t-online.de
 *
 * ATTINY45 @ 8 MHz
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *------------------------------------------------------------------------------
 */


#ifndef _INIT_H
#define _INIT_H


/*------------------------------------------------------------------------------
 * macros
 *------------------------------------------------------------------------------
 */

#ifndef _MACRO_PORT_DEFINITION
#define _MACRO_PORT_DEFINITION

// macro definition for port declarations (so that no bit manupulation is needed)
typedef char byte;
	struct test {
	  byte b0:1;
	  byte b1:1;
	  byte b2:1;
	  byte b3:1;
	  byte b4:1;
	  byte b5:1;
	  byte b6:1;
	  byte b7:1;
	} __attribute__((packed));

#define BIT(r,n) (((volatile struct test *)&r)->b##n)
#endif 


/*------------------------------------------------------------------------------
 * definitions
 *------------------------------------------------------------------------------
 */

// define port data output registers
//--------------------------------


// define port input registers
//--------------------------------



/*------------------------------------------------------------------------------
 * used functions
 *------------------------------------------------------------------------------
 */
void setup (void);


#endif //INIT_H
