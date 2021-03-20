/*------------------------------------------------------------------------------
 * RClights_4C.h
 * main for AeroLight_4C
 *
 * Copyright (c) 2015 Frank Scholl - MegaProjects@t-online.de
 *
 * ATtiny85
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *------------------------------------------------------------------------------
 */


#ifndef _MAIN_H
#define _MAIN_H

 /*------------------------------------------------------------------------------
 * definitions
 *------------------------------------------------------------------------------
 */

#define LEDS_PORT		PORTB	// Port for LEDs
#define LEDS_PORT_RD	PINB	// Port for LEDs
#define LED_NAVI		PB0		// LEDs for Navigation Lights
#define LED_ACL			PB1		// LEDs for ACL / Beacon
#define LED_STROBE		PB3		// LEDs for Strobe Lights
#define LED_LAND		PB4		// LEDs for Landing Lights

#define RC_PORT_RD		PINB	// Port for RC signal
#define RC_PIN			PB2		// Pin for RC signal

#define RCPULSE_MIN			 63		// minimum decoded RC pulse  63*16탎 = 1008탎
#define RCPULSE_MAX			126		// minimum decoded RC pulse 126*16탎 = 2016탎
#define RCPULSE_MINLIMIT	 50		// minimum valid   RC pulse  50*16탎 =  800탎
#define RCPULSE_MAXLIMIT	138		// minimum valid   RC pulse 138*16탎 = 2208탎

#define NUMBER_OF_CHANNELS 4	// do not change this. ATtiny85 can only handle up to 4 LED channels
/*
#define NAVI   0	// channel number for navi lights
#define ACL    1	// channel number for ACL lights
#define STROBE 2	// channel number for strobe lights
#define LAND   3	// channel number for landing lights
*/

// variables for used light patterns
typedef struct {
	bool		active;				// indicates that channel is activated
	uint8_t		ontime;				// index to table OntimeValues[] with values for LED on (each pulse) time in ms if activated
	uint16_t	ontimecount;		// counter for LED on time per pulse in ms
	uint8_t		offtimepulse;		// index to table OfftimeValues[] with values for LED off time between pulses in ms, 0=permanent on if activated
	uint16_t	offtimepulsecount;	// counter for LED off time between pulses in ms
	uint8_t		offtime;			// index to table OfftimeValues[] with values for LED off time after pulses in ms, 0=permanent on if activated
	uint16_t	offtimecount;		// counter for LED off time after pulses in ms
	uint16_t	timecount;			// counter for LED on/off  time in ms
	uint8_t		pulses;				// number of light pulses interrupted by offtimepulse until last offtime
	uint8_t		pulsecount;			// counter for pulses within 1 period
	uint8_t		ledpin;				// portpin where channel LED is connected to
} LightControl_t;

typedef struct {
	uint8_t		ontime;				// index to table OntimeValues[] with values for LED on (each pulse) time in ms if activated
	uint8_t		offtimepulse;		// index to table OfftimeValues[] with values for LED off time between pulses in ms, 0=permanent on if activated
	uint8_t		offtime;			// index to table OfftimeValues[] with values for LED off time after pulses in ms, 0=permanent on if activated
	uint8_t		pulses;				// number of light pulses interrupted by offtimepulse until last offtime
	uint8_t		ledpin;				// portpin where channel LED is connected to
} LightControl_Def_t;

/*------------------------------------------------------------------------------
 * Macro definitions
 *------------------------------------------------------------------------------
 */
//#define sbi(port,bit)  (port |= (1<<bit))   //set bit in port
//#define cbi(port,bit)  (port &= ~(1<<bit))  //clear bit in port

// Contrain() macro from Arduino:
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

/*------------------------------------------------------------------------------
 * FLASH constants
 *------------------------------------------------------------------------------
 */

/* list of time values [ms] for ontime parameter of a channel */
#define LEN_ONTIMEVALUES 8
extern const uint16_t OntimeValues[LEN_ONTIMEVALUES] PROGMEM;

/* list of time values [ms] for offtime parameter of a channel */
#define LEN_OFFTIMEVALUES 11
extern const uint16_t OfftimeValues[LEN_OFFTIMEVALUES] PROGMEM;


/*------------------------------------------------------------------------------
 * global variables
 *------------------------------------------------------------------------------
 */

extern volatile uint8_t		timecount_RCpulse;	// counter for RCpulse, 1 tic = 16,00탎
extern volatile bool		RCpulse_timeout;	// timed out on Timer overflow
extern volatile bool		RC_subch1;			// RC signal value subchannel 1
extern volatile bool		RC_subch2;			// RC signal value subchannel 2
extern volatile bool		RC_subch3;			// RC signal value subchannel 3
extern volatile bool		RC_subch4;			// RC signal value subchannel 4

extern volatile uint8_t button_release_count;	// 4ms timout counter for released button
extern volatile uint8_t button_pressed_count;	// 4ms timout counter for pressed button
extern volatile uint8_t button_state;			// show button state
#define BUTTON_NOT_AVAILABLE	0				// value for button state
#define BUTTON_PRESSED			1				// value for button state
#define BUTTON_RELEASED			2				// value for button state
extern volatile bool button_was_read;			// report button read state

extern volatile LightControl_t LightChannel[NUMBER_OF_CHANNELS];	// array pattern configurations per channel


// no 'volatile' qualifier for pointer target type
extern uint8_t    EE_pattern_select;		// saved pattern
extern uint8_t    EE_pattern_select_cp;		// saved pattern complement
extern uint8_t    EE_LEDspeed;				// saved LED speed
extern uint8_t    EE_LEDspeed_cp;			// saved LED speed complement


/*------------------------------------------------------------------------------
 * used functions
 *------------------------------------------------------------------------------
 */
//extern void switch_LED(uint8_t, uint8_t);
extern void flash (uint8_t, uint8_t, uint16_t, uint16_t, bool);


#endif //_MAIN_H


