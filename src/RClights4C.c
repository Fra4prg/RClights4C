/*------------------------------------------------------------------------------
 * RClights_4C.c
 *
 * Created: 04.11.2014
 *  Author: Frank
 *
 * main for RClights_4C
 *
 * Copyright (c) 2014 Frank Scholl - MegaProjects@t-online.de
 *
 * ATTINY85 @ 8 MHz
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *------------------------------------------------------------------------------
 *
 * 4 channel Light Control System for Model Remote Control
 * All 4 light channels can be switched individually by 1 RC channel using 2^4 = 16 Servo output levels
 * Generally these 4 channels are intended for
 *   - Navigation Lights
 *   - ACL / Beaver
 *   - Strobe Lights
 *   - Landing Lights
 * Program your RC transmitter so that 4 switches select 2^3, 2^2, 2^1 and 2^0 of full output (100%)
 *
 * ***** Attention: *****
 * Fusebit CKDIV8 is set by default. With this Bit the internal clock is divided by 8 from original 8MHz clock source!
 * see "6.2.6 Default Clock Source" on page 28 of the datasheet
 * It's also useful to set BODLEVEL to 2V7.
 *
 *------------------------------------------------------------------------------
 *
 * setting up Light channels via transmitter switches
 *
 * 1. To enter setup toggle switch 1 rapidly 5 times with 10 seconds after power on.
 *    All switches shall be off afterwards.
 *    Entering setup menu is indicated by flashing all LEDs twice
 * 2. Setup begins at channel 1.
 *    The current light pattern is shown on the selected active channel.
 *    To move to the next channel toggle switch 1.
 *    After channel 4 setup menu is exited.
 *    Exiting setup menu is indicated by flashing all LEDs twice
 * 3. To change the ontime of the pulses in the channel toggle switch 2 while switch 4 is in off position.
 *    You can select one of the predefined ontimes.
 * 4. To change the offtime after last pulse of the channel toggle switch 3 while switch 4 is in off position.
 *    You can select one of the predefined offtimes.
 *    If you select offtime=0 and number of pulses=1 then this channel is permanently on when activated.
 * 5. To change the number of pulses of the channel toggle switch 2 while switch 4 is in on position.
 *    You can select up to 5 pulses per channel.
 * 6. To change the offtime between pulses of the channel toggle switch 3 while switch 4 is in on position.
 *    You can select one of the predefined offtimes.
 *    If only 1 pulse is selected this parameter has no effect.
 *
 * That means switch 4 is like a modifier for switch 2 and 3 functions in setup.
 *
 *   principle of a light pattern with 3 pulses:
 *      ____        ____        ____                        ____
 *   __| p1 |______| p2 |______| p3 |______________________|    |__
 *     < on >< off >                <-------off (4.)------->
 *
 *
 *   | 1 | 2 | 3 | 4 |  <-- switch number
 *   +---+---+---+---+
 *   | c | 0 | 0 | 0 |  move to next channel
 *   | 0 | c | 0 | 0 |  change on time
 *   | 0 | 0 | c | 0 |  change off time after pulses
 *   | 0 | c | 0 | 1 |  change number of pulses
 *   | 0 | 0 | c | 1 |  change off time between pulses
 *
 *  c: toggle switch from off to on and back to off position
 *  0: switch is off
 *  1: switch is on
 *
 *------------------------------------------------------------------------------
 *
 * Timer usage:
 * --------------
 *
 * 8-bit Timer/Counter0 Compare Match A Interrupt
 *  1ms Interrupt for LED pattern changes
 *
 * PCINT2 on Port PB2 / Timer1:
 *  detect 1-2ms RC signal pulse
 *  on rising edge:  reset Timer1 for start of time measurement
 *  on falling edge: measure time of pulse
 *
 * 8-bit Timer/Counter1 Overflow Interrupt (max.period 4.096ms):
 *  detect missing pulse / missing RC signal
 *
 *------------------------------------------------------------------------------
 *
 * RC signal detection:
 *
 *      -->__<-- T=1..2ms             __                         __
 * _______|  |_______________________|  |_______________________|  |________
 *        ^ start Timer1             ^ start Timer1 ...
 *           ^ measure pulse length     ^ measure pulse length  ...
 *
 * Button detection:
 * ____________                _______________________
 *             |______________|
 *                          -->    <-- T>4ms => Timer1 Overflow
 *
 * (Timer1 Overflow) && (signal = low)   => button_pressed_count++, button_release_count=0, report pressed button
 * (Timer1 Overflow) && (signal = high)  => button_pressed_count=0, button_release_count++, report released button if it was read before
 * (PCINT2) rising or falling edge       => button_pressed_count=0, button_release_count=0, report no button available
 *
 *------------------------------------------------------------------------------
 */

 
//  Include files
#include "all_headers.h"

 /*------------------------------------------------------------------------------
 * definitions
 *------------------------------------------------------------------------------
 */

#define MAXPULSES	5	// maximum number of selectable pulses

#define	SETUPTIMEOUT			10000	// timout to enter setup menu after power on
#define MAX_TOGGLETIME_SW1		500		// max. time for toggling switch1 to enter setup menu
#define MIN_NO_SW1_TOGGLES		9		// min. toggles on and off switch1 to enter setup menu

#define FUNCTION_RCSIGNAL		true	// static action, no changes via RC signal or button possible
#define FUNCTION_BUTTON			true	// static action, no changes via RC signal or button possible
										// static action, no changes via RC signal or button possible

/*------------------------------------------------------------------------------
 * global variables
 *------------------------------------------------------------------------------
 */

volatile LightControl_t LightChannel[NUMBER_OF_CHANNELS];

volatile uint8_t	timecount_RCpulse  =   0;		// counter for RCpulse, 1 tic = 16,00µs
volatile bool		RCpulse_timeout = true;			// timed out on Timer overflow
volatile bool		RC_subch1 = false;				// RC signal value subchannel 1
volatile bool		RC_subch2 = false;				// RC signal value subchannel 2
volatile bool		RC_subch3 = false;				// RC signal value subchannel 3
volatile bool		RC_subch4 = false;				// RC signal value subchannel 4

volatile uint8_t button_release_count;				// 4ms timout counter for released button
volatile uint8_t button_pressed_count;				// 4ms timout counter for pressed button
volatile uint8_t button_state;						// show button state
volatile bool button_was_read;						// report button read state

/*------------------------------------------------------------------------------
 * FLASH constants
 *------------------------------------------------------------------------------
 */

/* program version in flash */
const char Program_Version[] PROGMEM = "RClights4C V15.12.29";

// list of time values [ms] for ontime parameter of a channel
// *** Attention: update line "#define LEN_ONTIMEVALUES 8" in header file !!!
const uint16_t OntimeValues[LEN_ONTIMEVALUES] PROGMEM =
{
	10,		// [0]
	20,		// [1]
	50,		// [2]
	100,	// [3]
	200,	// [4]
	300,	// [5]
	500,	// [6]
	1000	// [7]
};

// list of time values [ms] for offtime parameter of a channel
// *** Attention: update line "#define LEN_OFFTIMEVALUES 11" in header file !!!
const uint16_t OfftimeValues[LEN_OFFTIMEVALUES] PROGMEM =
{
	0,		// [0]
	10,		// [1]
	20,		// [2]
	50,		// [3]
	100,	// [4]
	300,	// [5]
	500,	// [6]
	1000,	// [7]
	1500,	// [8]
	2000,	// [9]
	3000	// [10]
};

// list of default values
const LightControl_Def_t LightChannel_Defaults[NUMBER_OF_CHANNELS] PROGMEM =
{
	{	// "Navi"
		4,	// ontime
		0,	// offtimepulse
		0,	// offtime
		1,	// pulses
		LED_NAVI	// LED pin
	},
	{	// "ACL"
		3,	// ontime
		3,	// offtimepulse
		8,	// offtime
		2,	// pulses
		LED_ACL	// LED pin
	},
	{	// "Strobe"
		2,	// ontime
		2,	// offtimepulse
		9,	// offtime
		3,	// pulses
		LED_STROBE	// LED pin
	},
	{	// "Land"
		4,	// ontime
		0,	// offtimepulse
		0,	// offtime
		1,	// pulses
		LED_LAND	// LED pin
	}
};

/*------------------------------------------------------------------------------
 * global EEPROM variables
 *------------------------------------------------------------------------------
 */
// no 'volatile' qualifier for pointer target type
LightControl_Def_t EEMEM LightChannel_EE[NUMBER_OF_CHANNELS];
LightControl_Def_t EEMEM LightChannel_EE_cp[NUMBER_OF_CHANNELS];

/*------------------------------------------------------------------------------
 * program
 *------------------------------------------------------------------------------
 */

/*------------------------------------------------------------------------------
 * flash
 *  @details  flash LEDs with interruption of normal pattern
 *  @param    LEDs to be flashed; number of flash pulses; ontime[MS]; offtime[MS]; set interrupt after return
 *  @return   -
 *------------------------------------------------------------------------------
 */
void flash (uint8_t leds, uint8_t number_of_pulses, uint16_t ontime, uint16_t offtime, bool set_int)
{
	int8_t i, portbsave;
	
	cli();	// stop normal interrupts
	
	// save current LED state
	portbsave = LEDS_PORT_RD;

	for(i=number_of_pulses;i>0;i--)
	{
		// LEDs on:
		LEDS_PORT  = (LEDS_PORT_RD | leds) | (1<<RC_PIN); // keep pullup at RC_PIN
		long_delay_ms(ontime);

		// LEDs off:
		LEDS_PORT  = (LEDS_PORT  & ~leds) | (1<<RC_PIN); // keep pullup at RC_PIN
		long_delay_ms(offtime);
	}
	
	// restore current LED state
	LEDS_PORT = portbsave | (1<<RC_PIN); // keep pullup at RC_PIN

	if (set_int)
	{
		sei();	// continue normal interrupts
	}
}


/*------------------------------------------------------------------------------
 * flashall
 *  @details  flash all LEDs subsequently and clear patterns optionally
 *  @param    option clear patterns
 *  @return   -
 *------------------------------------------------------------------------------
 */
void flashall(bool clearpatterns)
{
	uint8_t	ch;
	
	flash((1<<LED_NAVI), 1, 50, 100, false);
	flash((1<<LED_ACL), 1, 50, 100, false);
	flash((1<<LED_STROBE), 1, 10, 100, false);
	flash((1<<LED_LAND), 1, 50, 100, true);

	if (clearpatterns)
	{
		for(ch=0;ch<NUMBER_OF_CHANNELS;ch++)
		{
			LightChannel[ch].timecount = 0;
			LightChannel[ch].ontimecount = 0;
			LightChannel[ch].offtimepulsecount = 0;
			LightChannel[ch].offtimecount = 0;
			LightChannel[ch].pulsecount = 0;
		}
	}
}


/*------------------------------------------------------------------------------
 * setupmenu
 *  @details  setup patterns via switches pattern
 *  @param    -
 *  @return   -
 *------------------------------------------------------------------------------
 */
void setupmenu(void)
{
	uint8_t ch;
	
	// switch off all channels
	LightChannel[0].active = false;
	LightChannel[1].active = false;
	LightChannel[2].active = false;
	LightChannel[3].active = false;

	// test
	flashall(false);
	flashall(false);

	// wait until all switches are off at least 1 second
	while(RC_subch1 || RC_subch2 || RC_subch3 || RC_subch4);
	long_delay_ms(200);
	
	// for all 4 channels do
	ch=0;
	while(ch<NUMBER_OF_CHANNELS)
	{
		LightChannel[ch].active = true;	// display current channel
		// if switch1 is on
		if(RC_subch1)
		{
			LightChannel[ch].active = false;	// hide current channel
			
			// store all values of this channel in EEPROM
			if (LightChannel[ch].pulses!=eeprom_read_byte(&LightChannel_EE[ch].pulses))
			{
				eeprom_write_byte(&LightChannel_EE[ch].pulses,LightChannel[ch].pulses);
				eeprom_write_byte(&LightChannel_EE_cp[ch].pulses,~LightChannel[ch].pulses);
			}
			if (LightChannel[ch].ontime!=eeprom_read_byte(&LightChannel_EE[ch].ontime))
			{
				eeprom_write_byte(&LightChannel_EE[ch].ontime,LightChannel[ch].ontime);
				eeprom_write_byte(&LightChannel_EE_cp[ch].ontime,~LightChannel[ch].ontime);
			}
			if (LightChannel[ch].offtimepulse!=eeprom_read_byte(&LightChannel_EE[ch].offtimepulse))
			{
				eeprom_write_byte(&LightChannel_EE[ch].offtimepulse,LightChannel[ch].offtimepulse);
				eeprom_write_byte(&LightChannel_EE_cp[ch].offtimepulse,~LightChannel[ch].offtimepulse);
			}
			if (LightChannel[ch].offtime!=eeprom_read_byte(&LightChannel_EE[ch].offtime))
			{
				eeprom_write_byte(&LightChannel_EE[ch].offtime,LightChannel[ch].offtime);
				eeprom_write_byte(&LightChannel_EE_cp[ch].offtime,~LightChannel[ch].offtime);
			}

			// move to next channel
			ch++;
			// wait until switch1 is off
			while(RC_subch1);
			long_delay_ms(200);
		}
		else
		{
			// if switch2 is on
			if (RC_subch2)
			{
				// if switch4 is off
				if (!RC_subch4)
				{
					// change on time
					LightChannel[ch].ontime++;
					if (LightChannel[ch].ontime>=LEN_ONTIMEVALUES)
					{
						LightChannel[ch].ontime=0;
					}
				}
				else
				{
					// change number of pulses
					LightChannel[ch].pulses++;
					if (LightChannel[ch].pulses>MAXPULSES)
					{
						LightChannel[ch].pulses=1;
					}
				}
				// wait until switch2 is off
				while(RC_subch2);
				long_delay_ms(200);
			}
			else
			{
				// if switch3 is on
				if (RC_subch3)
				{
					// if switch4 is off
					if (!RC_subch4)
					{
						// change off time after pulses
						LightChannel[ch].offtime++;
						if (LightChannel[ch].offtime>=LEN_OFFTIMEVALUES)
						{
							LightChannel[ch].offtime=0;
						}
					}
					else
					{
						// change off time between pulses
						LightChannel[ch].offtimepulse++;
						if (LightChannel[ch].offtimepulse>=LEN_OFFTIMEVALUES)
						{
							LightChannel[ch].offtimepulse=0;
						}
					}
					//       wait until switch3 is off
					while(RC_subch3);
					long_delay_ms(200);
				}
			}
		}
	}
	// exiting menu
	flashall(false);
	flashall(false);
}


/*------------------------------------------------------------------------------
 * setup_patterns
 *  @details  read values from EEPROM and initialize if complement doesn't match
 *  @param    -
 *  @return   -
 *------------------------------------------------------------------------------
 */
void setup_patterns()
{
	uint8_t rd_EE, rd_EE_cp, ch;
	bool		eeprom_unplausible=false;

	// setup initial values for all channels
	for (ch=0;ch<NUMBER_OF_CHANNELS;ch++)
	{
		// number of pulses
		rd_EE	=eeprom_read_byte(&LightChannel_EE[ch].pulses);
		rd_EE_cp=eeprom_read_byte(&LightChannel_EE_cp[ch].pulses);
		if ( ((rd_EE+rd_EE_cp) != 255) || (rd_EE > MAXPULSES )) // check redundancy by complement
		{
			LightChannel[ch].pulses=pgm_read_byte(&(LightChannel_Defaults[ch].pulses));
			eeprom_write_byte(&LightChannel_EE[ch].pulses,LightChannel[ch].pulses);
			eeprom_write_byte(&LightChannel_EE_cp[ch].pulses,~LightChannel[ch].pulses);
			eeprom_unplausible=true;
		}
		else
		{
			LightChannel[ch].pulses=rd_EE;
		}
	
		// ontime
		rd_EE	=eeprom_read_byte(&LightChannel_EE[ch].ontime);
		rd_EE_cp=eeprom_read_byte(&LightChannel_EE_cp[ch].ontime);
		if ( ((rd_EE+rd_EE_cp) != 255) || (rd_EE > LEN_ONTIMEVALUES )) // check redundancy by complement
		{
			LightChannel[ch].ontime=pgm_read_byte(&(LightChannel_Defaults[ch].ontime));
			eeprom_write_byte(&LightChannel_EE[ch].ontime,LightChannel[ch].ontime);
			eeprom_write_byte(&LightChannel_EE_cp[ch].ontime,~LightChannel[ch].ontime);
			eeprom_unplausible=true;
		}
		else
		{
			LightChannel[ch].ontime=rd_EE;
		}

		// offtimepulses
		rd_EE	=eeprom_read_byte(&LightChannel_EE[ch].offtimepulse);
		rd_EE_cp=eeprom_read_byte(&LightChannel_EE_cp[ch].offtimepulse);
		if ( ((rd_EE+rd_EE_cp) != 255) || (rd_EE > LEN_OFFTIMEVALUES )) // check redundancy by complement
		{
			LightChannel[ch].offtimepulse=pgm_read_byte(&(LightChannel_Defaults[ch].offtimepulse));
			eeprom_write_byte(&LightChannel_EE[ch].offtimepulse,LightChannel[ch].offtimepulse);
			eeprom_write_byte(&LightChannel_EE_cp[ch].offtimepulse,~LightChannel[ch].offtimepulse);
			eeprom_unplausible=true;
		}
		else
		{
			LightChannel[ch].offtimepulse=rd_EE;
		}

		// offtime
		rd_EE	=eeprom_read_byte(&LightChannel_EE[ch].offtime);
		rd_EE_cp=eeprom_read_byte(&LightChannel_EE_cp[ch].offtime);
		if ( ((rd_EE+rd_EE_cp) != 255) || (rd_EE > LEN_OFFTIMEVALUES )) // check redundancy by complement
		{
			LightChannel[ch].offtime=pgm_read_byte(&(LightChannel_Defaults[ch].offtime));
			eeprom_write_byte(&LightChannel_EE[ch].offtime,LightChannel[ch].offtime);
			eeprom_write_byte(&LightChannel_EE_cp[ch].offtime,~LightChannel[ch].offtime);
			eeprom_unplausible=true;
		}
		else
		{
			LightChannel[ch].offtime=rd_EE;
		}

		// rest of structure
		LightChannel[ch].active=false;
		LightChannel[ch].ledpin=pgm_read_byte(&LightChannel_Defaults[ch].ledpin);
	}
	if (eeprom_unplausible)
	{
		flash((1<<LED_LAND), 5, 50, 100, true);
		long_delay_ms(300);
	}
}


 /*-----------------------------------------------------------------------------
 * Pin Change Interrupt Request 0
 *  @details  -
 *  @param    -
 *  @return   -
 *------------------------------------------------------------------------------
 */
ISR (PCINT0_vect) 
{
	// reset button detection (set by 4ms timeout)
	button_release_count = 0;
	button_pressed_count = 0;

	//GIFR |= ( 1 << PCIF ); // clear interrupt flag
	if (RC_PORT_RD & (1<<RC_PIN))  // detect rising edge
	{
		// reset Timer1
		TCNT1=0;
		RCpulse_timeout=false;
	}
	else // detect falling edge
	{
		// decode subchannels from RC signal
		// pulse length is Timer1 value
		timecount_RCpulse=TCNT1;

		if((timecount_RCpulse>=RCPULSE_MINLIMIT) && (timecount_RCpulse<=RCPULSE_MAXLIMIT) && !(RCpulse_timeout)) // check for valid pulse
		{
			button_state = BUTTON_NOT_AVAILABLE; // valid RC signal excludes button
			
			if(timecount_RCpulse<RCPULSE_MIN) // check for used range
			{
				// below all subchannels are off
				RC_subch1 = true;
				RC_subch2 = true;
				RC_subch3 = true;
				RC_subch4 = true;
			}
			else
			{
				if((timecount_RCpulse>RCPULSE_MAX)) // check for used range
				{
					// above all subchannels are on
					RC_subch1 = false;
					RC_subch2 = false;
					RC_subch3 = false;
					RC_subch4 = false;
				}
				else
				{
					// decode subchannels
					timecount_RCpulse = timecount_RCpulse - RCPULSE_MIN;
					RC_subch1 = (timecount_RCpulse & (1<<5));
					RC_subch2 = (timecount_RCpulse & (1<<4));
					RC_subch3 = (timecount_RCpulse & (1<<3));
					RC_subch4 = (timecount_RCpulse & (1<<2));
				}
			}
		}
	}
}


/*------------------------------------------------------------------------------
 * main
 *  @details  -
 *  @param    -
 *  @return   -
 *------------------------------------------------------------------------------
 */
int main(void)
{
	bool		switch1_current, switch1_previous;
	uint16_t	switch1_time_last_change;
	uint8_t		switch1_toggle_count;
	
	setup();  // initialize hardware
	
	setup_patterns();	// read pattern configuration from EEPROM and initialize
	
	flashall(true);	// test all LEDs at startup

	// reset setup entrance
	switch1_current = false;
	switch1_previous = false;
	switch1_time_last_change=0;
	switch1_toggle_count=0;
    
	while(1) // main loop
    {
/*
		// ------------------------------------------------------------
		// static action, no changes via RC signal or button possible
		LightChannel[0].active = true;
		LightChannel[1].active = true;
		LightChannel[2].active = true;
		LightChannel[3].active = true;
		while(1) {}
		// ------------------------------------------------------------
*/		
		if ((button_state==BUTTON_PRESSED) & (!button_was_read))
		{
			
			// switch on all LightChannels subsequently
			if (!RC_subch1)
			{
				RC_subch1 = true;
			}
			else
			{
				if (!RC_subch2)
				{
					RC_subch2 = true;
				}
				else
				{
					if (!RC_subch3)
					{
						RC_subch3 = true;
					}
					else
					{
						if (!RC_subch4)
						{
							RC_subch4 = true;
						}
						else
						{
							// after all on switch all off
							RC_subch1 = false;
							RC_subch2 = false;
							RC_subch3 = false;
							RC_subch4 = false;
						}
					}
				}
		
			}
			button_was_read=true;	// report button read to ISR
		}
		LightChannel[0].active = RC_subch1;
		LightChannel[1].active = RC_subch2;
		LightChannel[2].active = RC_subch3;
		LightChannel[3].active = RC_subch4;
		
		// jump into setup menu when switch 1 is changed rapidly within 10 seconds after power on
		if (TimeCounter<SETUPTIMEOUT)
		{
			switch1_previous=switch1_current;
			switch1_current=LightChannel[0].active;
			if (switch1_current==switch1_previous)	// count time if switch was not changed
			{
				if(switch1_time_last_change<65535)
				{
					switch1_time_last_change++;
				}
			}
			else
			{
				if (switch1_time_last_change<MAX_TOGGLETIME_SW1)	// if switch changed rapidly
				{
					switch1_toggle_count++;
					if (switch1_toggle_count>=MIN_NO_SW1_TOGGLES)	// if toggled enough then enter setup menu
					{
						setupmenu();
					}
				}
				else
				{
					switch1_toggle_count=0;
				}
				switch1_time_last_change=0;
			}
		
			_delay_ms(1);
		}
		
	} // end main loop
}