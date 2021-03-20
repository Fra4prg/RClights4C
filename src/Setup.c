/*------------------------------------------------------------------------------
 * Setup.c
 * generic initialisation routines for AeroLight_4C
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

 
 //  Include files
#include "all_headers.h"


/*------------------------------------------------------------------------------
 * Included Functions and ISR
 *  @details  -
 *  @param    -
 *  @return   -
 *------------------------------------------------------------------------------
 */

 
/*------------------------------------------------------------------------------
 * global variables
 *------------------------------------------------------------------------------
 */




/*------------------------------------------------------------------------------
 * Configure Ports (set direction)
 *  @details  -
 *  @param    -
 *  @return   -
 *------------------------------------------------------------------------------
 */
void config_ports (void)
{
	
	//Set Direction Registers (Input/Output) default set to Input (0x00) 
	//------------------------------------------------------------------
	
	// *** configure PORTB ***
	// Set output direction registers
	// PB.0 = (out, low)		LEDs for Navigation Lights
	// PB.1 = (out, low)		LEDs for ACL / Beacon
	// PB.2 = (in, Pullup)		RC signal
	// PB.3 = (out, low)		LEDs for Strobe Lights
	// PB.4 = (out, low)		LEDs for Landing Lights
	// PB.5 = (RESET in)
	//
	// Set input pullups
	PORTB = (0<<PB0)  | (0<<PB1)  | (1<<PB2)  | (0<<PB3)  | (0<<PB4)  | (0<<PB5);
	// Set directions
	DDRB  = (1<<DDB0) | (1<<DDB1) | (0<<DDB2) | (1<<DDB3) | (1<<DDB4) | (0<<DDB5);

}


/*------------------------------------------------------------------------------
 * Configure External Interrupts
 *  @details  -
 *  @param    -
 *  @return   -
 *------------------------------------------------------------------------------
 */
void config_extint (void)
{
	// *** MCU Control Register ***
	// SC01, ISC00: Interrupt Sense Control 0 Bit 1 and Bit 0
	//MCUCR = (0<<BODS) | (0<<PUD) | (0<<SE) | (0<<SM1) | (0<<SM0) | (0<<BODSE)  | (0<<ISC01)  | (0<<ISC00); // complete register
	//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx only change these bits (other bits used for different function)
	//                                               v            v          --> only bits used in this unit
	MCUCR = (MCUCR & ~((1<<ISC01) | (1<<ISC00))) | ((1<<ISC01) | (0<<ISC00));

	// *** GIMSK – General Interrupt Mask Register ***
	// INT0: External Interrupt Request 0 Enable
	// PCIE: Pin Change Interrupt Enable
	GIMSK = (0<<INT0) | (1<<PCIE); // complete register

	// *** GIFR – General Interrupt Flag Register ***
	// INTF0: External Interrupt Flag 0
	// PCIF: Pin Change Interrupt Flag
	//GIFR = (0<<INTF0) | (0<<PCIF); // complete register

	// *** PCMSK – Pin Change Mask Register ***
	// PCINT5:0: Pin Change Enable Mask 5:0
	PCMSK = (0<<PCINT5) | (0<<PCINT4) | (0<<PCINT3) | (1<<PCINT2) | (0<<PCINT1) | (0<<PCINT0); // complete register


}


/*------------------------------------------------------------------------------
 * setup uC
 *  @details  -
 *  @param    -
 *  @return   -
 *------------------------------------------------------------------------------
 */
void setup (void)
{	
	//Init uC peripheral
	//-------------------
	config_ports();			// this configures the I/O ports

	TimerInit();
	
	config_extint();

	// reset button detection (set by 4ms timeout)
	button_release_count = 0;
	button_pressed_count = 0;
	button_was_read = true;
	button_state = BUTTON_NOT_AVAILABLE;

		
		// *** get saved values from EEPROM ***

//		flash(0b10000000, 10, 20, 50, false);  // =====================
//		long_delay_ms(1000);

/* compare complement does not work at first instance !!!!!!!!!!!!!?????????
		rd_EE = eeprom_read_byte(&EE_pattern_select);
		rd_EE_cp = eeprom_read_byte(&EE_pattern_select_cp);
		if (rd_EE == (~rd_EE_cp))  // check redundancy by complement
		{
			check=0; // dummy
		}


//		flash(0b10000000, 10, 20, 50, false);  // =====================
//		long_delay_ms(1000);

		LEDspeed = constrain(eeprom_read_byte(&EE_LEDspeed),SPEED_MIN,SPEED_MAX);
		eeprom_write_byte ( &EE_LEDspeed, LEDspeed);
		eeprom_write_byte ( &EE_LEDspeed_cp, ~LEDspeed);
*/

		// enable interrupts
		sei();
		
//		long_delay_ms(100);
}


