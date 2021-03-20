/*------------------------------------------------------------------------------
 * Timer_ATtiny-25-45-85.h
 * Timer routines
 *
 * Copyright (c) 2012 Frank Scholl - MegaProjects@t-online.de
 *
 * ATtiny25/45/85
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *------------------------------------------------------------------------------
 */

 
#ifndef _TIMER_H
#define _TIMER_H


 /*------------------------------------------------------------------------------
 * definitions
 *------------------------------------------------------------------------------
 */

#if !defined (F_CPU)
#define F_CPU 8000000
#endif


 /*------------------------------------------------------------------------------
 * globale variables
 *------------------------------------------------------------------------------
 */

extern volatile uint16_t		TimeCounter;

/*------------------------------------------------------------------------------
 * used functions (without ISRs)
 *------------------------------------------------------------------------------
 */
extern void TimerInit(void);
extern void long_delay_ms(uint16_t);


#endif //_TIMER_H


