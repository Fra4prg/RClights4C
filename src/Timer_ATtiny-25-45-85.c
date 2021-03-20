/*------------------------------------------------------------------------------
 * Timer_ATtiny-25-45-85.c
 * Timer routines
 *
 * Copyright (c) 2014 Frank Scholl - MegaProjects@t-online.de
 *
 * ATtiny25/45/85
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *------------------------------------------------------------------------------
 */

 
//  Include files
#include "all_headers.h"

// define clock if not done so far
#ifndef F_CPU
#error  please define clock in *timer*.h
#endif
// ***** Attention: *****
// Fusebit CKDIV8 is set by default. With this Bit the internal clock is divided by 8 from original 8MHz clock source!
// see "6.2.6 Default Clock Source" on page 28 of the datasheet


 /*------------------------------------------------------------------------------
 * global variables
 *------------------------------------------------------------------------------
 */

volatile uint16_t		TimeCounter = 0;			// counter for ms, used for measuring setup-timeout (16 bit is enough for 65 seconds)


/*------------------------------------------------------------------------------
 * longer delays
 *  @details  max. time for _delay_ms() is limited to
 *            262.14 ms / F_CPU [MHz] (e.g.: 262.1 / 3.6864 = max. 71 ms)
 *            Hence the delay routine is called multiple for getting longer delays
 *            The additional check of the loop condition makes the delay a bit unexactly
 *            This routine does not used Timer functions.
 *  @param    number of milliseconds
 *  @return   -
 *------------------------------------------------------------------------------
 */
void long_delay_ms(uint16_t ms)
{
	for(;ms>0;ms--) _delay_ms(1);
}


/*------------------------------------------------------------------------------
 * Timer Initialisation
 *  @details  -
 *  @param    -
 *  @return   -
 *------------------------------------------------------------------------------
 */
void TimerInit(void)
{
	// **************************** prescaler settings: ****************************
	// Timer0:
	// fQ=1MHz:
	//   CSn2:0 = %000 -->  No clock source (Timer/Counter stopped)
	//   CSn2:0 = %001 -->  1MHz/   1 =  1000000,00Hz   --> 1 tic=   1,00µs  -> max.8bit =    0,256ms
	//   CSn2:0 = %010 -->  1MHz/   8 =   125000,00Hz   --> 1 tic=   8,00µs  -> max.8bit =    2,048ms
	//   CSn2:0 = %011 -->  1MHz/  64 =    15625,00Hz   --> 1 tic=  64,00µs  -> max.8bit =   16,384ms
	//   CSn2:0 = %100 -->  1MHz/ 256 =     3906,25Hz   --> 1 tic= 256,00µs  -> max.8bit =   65,536ms
	//   CSn2:0 = %101 -->  1MHz/1024 =      976,5625Hz --> 1 tic=1024,00µs  -> max.8bit =  262,144ms
	//   CSn2:0 = %110 External clock source on T0 pin. Clock on falling edge.
	//   CSn2:0 = %111 External clock source on T0 pin. Clock on rising edge.

	// fQ=8MHz:
	//   CSn2:0 = %000 -->  No clock source (Timer/Counter stopped)
	//   CSn2:0 = %001 -->  8MHz/   1 =  8000000,00Hz --> 1 tic=   125ns  -> max.8bit =  0,03200ms
	//   CSn2:0 = %010 -->  8MHz/   8 =  1000000,00Hz --> 1 tic=  1,00µs  -> max.8bit =  0,25600ms
	// * CSn2:0 = %011 -->  8MHz/  64 =   125000,00Hz --> 1 tic=  8,00µs  -> max.8bit =  2,04800ms
	//   CSn2:0 = %100 -->  8MHz/ 256 =    31250,00Hz --> 1 tic= 32,00µs  -> max.8bit =  8,19200ms
	//   CSn2:0 = %101 -->  8MHz/1024 =     7812,50Hz --> 1 tic=128,00µs  -> max.8bit = 32,76800ms
	//   CSn2:0 = %110 External clock source on T0 pin. Clock on falling edge.
	//   CSn2:0 = %111 External clock source on T0 pin. Clock on rising edge.

	// *********************** 8-bit Timer/Counter0 with PWM ***********************
	// (datasheet page 67)

	// usage: <note usage here>

	// *** GTCCR – General Timer/Counter Control Register***
	// TSM: Timer/Counter Synchronization Mode
	// PWM1B:  see Timer1
	// COM1B1: see Timer1
	// COM1B0: see Timer1
	// FOC1B:  see Timer1
	// FOC1A:  see Timer1
	// PSR1:   see Timer1
	// PSR0: Prescaler Reset Timer/Counter0
//	GTCCR = (0<<TSM) | (0<<PWM1B) | (0<<COM1B1) | (0<<COM1B0) | (0<<FOC1B) | (0<<FOC1A) | (0<<PSR1) | (0<<PSR0); // complete register
	//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx only change these bits (other bits used for different function)
//	GTCCR = (GTCCR & ~((1<<TSM) | (1<<PSR0))) | ((0<<TSM) | (0<<PSR0)); // only bits used in this unit

	// *** TCCR0A – Timer/Counter Control Register A***
	//
	// Table 11-2. Compare Output Mode, non-PWM
	//  COM0A1/COM0A0
	//  COM0B1/COM0B0
	//		%00 = Normal port operation, OC1A/OC1B disconnected.
	//		%01 = Toggle OC0A/OC0B on Compare Match.
	//		%10 = Clear  OC0A/OC0B on Compare Match (Set output to low level).
	//		%11 = Set    OC0A/OC0B on Compare Match (Set output to high level).
	//
	// Table 11-3. Compare Output Mode, Fast PWM
	//  COM0A1/COM0A0
	//  COM0B1/COM0B0
	//	*	%00 = Normal port operation, OC0A/OC0B disconnected.
	//		%01 = reserved.
	//		%10 = Clear OC0A/OC0B on Compare Match, set   OC0A/OC0B at BOTTOM (non-inverting mode)
	//		%11 = Set   OC0A/OC0B on Compare Match, clear OC0A/OC0B at BOTTOM (inverting mode)
	//
	// Table 11-4. Compare Output Mode, Phase Correct Correct PWM
	//	*	%00 = Normal port operation, OC0A/OC0B disconnected.
	//		%01 = reserved.
	//		%10 = Clear OC0A/OC0B on Compare Match when upcounting. Set   OC0A/OC0B on Compare Match when downcounting.
	//		%11 = Set   OC0A/OC0B on Compare Match when upcounting. Clear OC0A/OC0B on Compare Match when downcounting.
	//
	// Table 11-5. Waveform Generation Modes
	// Mode  WGM    Mode of                               Update of  TOV1 Flag
	//       02:00  Operation                     TOP     OCR1x at   Set on
	//  0     000   Normal                        0xFF    Immediate  MAX
	//  1     001   PWM, Phase Correct            0xFF    TOP        BOTTOM
	//  2  *  010   CTC (Clear Timer on Compare)  OCR1A   Immediate  MAX
	//  3     011   Fast PWM                      0xFF    BOTTOM     MAX
	//  4     100   (Reserved)
	//  5     101   PWM, Phase Correct            OCR1A   TOP        BOTTOM
	//  7     110   (Reserved)                    –       –          –
	//  8     111   Fast PWM                      OCR1A   BOTTOM     TOP
	TCCR0A = (0<<COM0A1) | (0<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (1<<WGM01) | (0<<WGM00);


	// *** TCCR0B – Timer/Counter Control Register B ***
	// FOC0A: Force Output Compare A
	// FOC0B: Force Output Compare B
	// CS02..0 example values see above
	//	Table 11-6. Clock Select Bit Description
	//	CS02..0 Description
	//	%000 No clock source (Timer/Counter stopped)
	//	%001 clkI/O	/     (No prescaling)
	//	%010 clkI/O	/   8 (From prescaler)
	//	%011 clkI/O	/  64 (From prescaler)
	//	%100 clkI/O	/ 256 (From prescaler)
	//	%101 clkI/O	/1024 (From prescaler)
	//	%110 External clock source on T0 pin. Clock on falling edge.
	//	%111 External clock source on T0 pin. Clock on rising edge.
	//
	//  Fast PWM                    -->  f(OCnxPWM)   = f(CLK_IO) / (N * 256)
	//  Phase Correct Correct PWM   -->  f(OCnxPCPWM) = f(CLK_IO) / (N * 510)
	//     [The N variable represents the prescale factor (1, 8, 64, 256, or 1024)]
	//
	TCCR0B = (0<<FOC0A) | (0<<FOC0B) | (0<<WGM02) | (0<<CS02) | (1<<CS01) | (1<<CS00);
	
	// *** TCNT0 – Timer/Counter Register ***
//	TCNT0 = 0;

	// *** OCR0A – Output Compare Register A ***
	OCR0A = 125;	// 1ms

	// *** OCR0B – Output Compare Register B ***
//	OCR0B = 100;

	// *** TIMSK – Timer/Counter 0 Interrupt Mask Register ***
	// OCIE1B: see Timer1
	// OCIE1A: see Timer1
	// OCIE0B: Timer/Counter0 Output Compare Match B Interrupt Enable
	// OCIE0A: Timer/Counter0 Output Compare Match A Interrupt Enable
	// TOIE1:  see Timer1
	// TOIE0:  Timer/Counter0 Overflow Interrupt Enable
//	TIMSK = (0<<OCIE1A) | (0<<OCIE1B) | (0<<OCIE0A) | (0<<OCIE0B) | (0<<TOIE1) | (0<<TOIE0); // complete register
	//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx only change these bits (other bits used for different function)
	//                                                              v             v             v          --> only bits used in this unit
	TIMSK = (TIMSK & ~((1<<OCIE0A) | (1<<OCIE0B) | (1<<TOIE0))) | ((1<<OCIE0A) | (0<<OCIE0B) | (0<<TOIE0));

	// *** TIFR – Timer/Counter 0 Interrupt Flag Register ***
	// OCF1B: see Timer1
	// OCF1A: see Timer1
	// OCF0B: Output Compare Flag 0 B
	// OCF0A: Output Compare Flag 0 A
	// TOV1:  see Timer1
	// TOV0:  Timer/Counter0 Overflow Flag
//	TIFR = (0<<OCF1B) | (0<<OCF1A) | (0<<OCF0B) | (0<<OCF0A) | (0<<TOV1) | (0<<TOV0); // complete register
	//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx only change these bits (other bits used for different function)
	//                                                         v            v            v          --> only bits used in this unit
//	TIFR = (TIFR & ~((1<<OCF0B) | (1<<OCF0A) | (1<<TOV0))) | ((0<<OCF0B) | (0<<OCF0A) | (0<<TOV0)); 


	// *************************** 8-bit Timer/Counter1 ***************************
	// (datasheet page 86)
	
	// Timer1:
	// fQ=8MHz:
	//   CSn3:0 = %0000 -->  No clock source (Timer/Counter stopped)
	//   CSn3:0 = %0001 -->  8MHz/    1 =  8000000,00Hz    --> 1 tic=    125ns  -> max.8bit =    0,032ms
	//   CSn3:0 = %0010 -->  8MHz/    2 =  4000000,00Hz    --> 1 tic=    250ns  -> max.8bit =    0,064ms
	//   CSn3:0 = %0011 -->  8MHz/    4 =  2000000,00Hz    --> 1 tic=    500ns  -> max.8bit =    0,128ms
	//   CSn3:0 = %0100 -->  8MHz/    8 =  1000000,00Hz    --> 1 tic=   1,00µs  -> max.8bit =    0,256ms
	//   CSn3:0 = %0101 -->  8MHz/   16 =   500000,00Hz    --> 1 tic=   2,00µs  -> max.8bit =    0,512ms
	//   CSn3:0 = %0110 -->  8MHz/   32 =   250000,00Hz    --> 1 tic=   4,00µs  -> max.8bit =    1,024ms
	//   CSn3:0 = %0111 -->  8MHz/   64 =   125000,00Hz    --> 1 tic=   8,00µs  -> max.8bit =    2,048ms
	// * CSn3:0 = %1000 -->  8MHz/  128 =    62500,00Hz    --> 1 tic=  16,00µs  -> max.8bit =    4,096ms
	//   CSn3:0 = %1001 -->  8MHz/  256 =    31250,00Hz    --> 1 tic=  32,00µs  -> max.8bit =    8,192ms
	//   CSn3:0 = %1010 -->  8MHz/  512 =    15625,00Hz    --> 1 tic=  64,00µs  -> max.8bit =   16,384ms
	//   CSn3:0 = %1011 -->  8MHz/ 1024 =     7812,50Hz    --> 1 tic= 128,00µs  -> max.8bit =   32,768ms
	//   CSn3:0 = %1100 -->  8MHz/ 2048 =     3906,25Hz    --> 1 tic= 256,00µs  -> max.8bit =   65,536ms
	//   CSn3:0 = %1101 -->  8MHz/ 4096 =     1953,125Hz   --> 1 tic= 512,00µs  -> max.8bit =  131,072ms
	//   CSn3:0 = %1110 -->  8MHz/ 8192 =      976,5625Hz  --> 1 tic=1024,00µs  -> max.8bit =  262,144ms
	//   CSn3:0 = %1111 -->  8MHz/16384 =      488,28125Hz --> 1 tic=2048,00µs  -> max.8bit =  524,288ms

	// usage: <note usage here>

	// Table 12-1. Compare Mode Select in PWM Mode
	//  COM11/COM10
	//	*	%00 =  OC1x not connected.	
	//            -OC1x not connected.
	//		%01 =  OC1x cleared on compare match. Set whenTCNT1 = $00.
	//            -OC1x set on compare match. Cleared when TCNT1 = $00.
	//		%10 =  OC1x cleared on compare match. Set when TCNT1 = $00.
	//            -OC1x not connected.
	//		%11 =  OC1x Set on compare match. Cleared when TCNT1= $00.
	//            -OC1x not connected.
	//
	// Table 12-2. PWM Outputs OCR1x = $00 or OCR1C, x = A or B
	// COM1x1/COM1x0 OCR1x Output OC1x Output OC1x
	//      %01 =     $00      L       H
	//      %01 =     OCR1C    H       L
	//      %10 =     $00      L       Not connected.
	//      %10 =     OCR1C    H       Not connected.
	//      %11 =     $00      H       Not connected.
	//      %11 =     OCR1C    L       Not connected.
	//
	//	Table 12-3. Timer/Counter1 Clock Prescale Select in the Asynchronous Mode
	//	PWM Frequency Clock Selection CS13:CS10 OCR1C RESOLUTION
	//	 20 kHz       PCK/16          %0101     199     7.6
	//	 30 kHz       PCK/16          %0101     132     7.1
	//	 40 kHz       PCK /8          %0100     199     7.6
	//	 50 kHz       PCK /8          %0100     159     7.3
	//	 60 kHz       PCK /8          %0100     132     7.1
	//	 70 kHz       PCK /4          %0011     228     7.8
	//	 80 kHz       PCK /4          %0011     199     7.6
	//	 90 kHz       PCK /4          %0011     177     7.5
	//	100 kHz       PCK /4          %0011     159     7.3
	//	110 kHz       PCK /4          %0011     144     7.2
	//	120 kHz       PCK /4          %0011     132     7.1
	//	130 kHz       PCK /2          %0010     245     7.9
	//	140 kHz       PCK /2          %0010     228     7.8
	//	150 kHz       PCK /2          %0010     212     7.7
	//	160 kHz       PCK /2          %0010     199     7.6
	//	170 kHz       PCK /2          %0010     187     7.6
	//	180 kHz       PCK /2          %0010     177     7.5
	//	190 kHz       PCK /2          %0010     167     7.4
	//	200 kHz       PCK /2          %0010     159     7.3
	//	250 kHz       PCK             %0001     255     8.0
	//	300 kHz       PCK             %0001     212     7.7
	//	350 kHz       PCK             %0001     182     7.5
	//	400 kHz       PCK             %0001     159     7.3
	//	450 kHz       PCK             %0001     141     7.1
	//	500 kHz       PCK             %0001     127     7.0
	
	// *** TCCR1 – Timer/Counter1 Control Register ***
	// CTC1: Clear Timer/Counter on Compare Match
	// PWM1A: Pulse Width Modulator A Enable
	// COM1A1, COM1A0: Comparator A Output Mode, Bits 1 and 0
	//  	Table 12-4. Comparator A Mode Select
	//  	COM1A1/COM1A0 Description
	//  *	%00 Timer/Counter Comparator A disconnected from output pin OC1A.
	//  	%01 Toggle the OC1A output line.
	//  	%10 Clear the OC1A output line.
	//  	%11 Set the OC1A output line
	// CS13, CS12, CS11, CS10: Clock Select Bits 3, 2, 1, and 0
	//  	Table 12-5. Timer/Counter1 Prescale Select
	//  	CS13..0  Asynchronous    Synchronous
	//  	         Clocking Mode   Clocking Mode
	//  	%0000    T/C1 stopped    T/C1 stopped
	//  	%0001    PCK             CK
	//  	%0010    PCK/2           CK/2
	//  	%0011    PCK/4           CK/4
	//  	%0100    PCK/8           CK/8
	//  	%0101    PCK/16          CK/16
	//  	%0110    PCK/32          CK/32
	//  	%0111    PCK/64          CK/64
	//  *	%1000    PCK/128         CK/128
	//  	%1001    PCK/256         CK/256
	//  	%1010    PCK/512         CK/512
	//  	%1011    PCK/1024        CK/1024
	//  	%1100    PCK/2048        CK/2048
	//  	%1101    PCK/4096        CK/4096
	//  	%1110    PCK/8192        CK/8192
	//  	%1111    PCK/16384       CK/16384
	//  PWM -->  f(OCnxPWM)   = f(CLK_IO) / (N * 256)
	TCCR1 = (0<<CTC1) | (0<<PWM1A) | (0<<COM1A1) | (0<<COM1A0) | (1<<CS13) | (0<<CS12) | (0<<CS11) | (0<<CS10);
	
	// *** GTCCR – General Timer/Counter Control Register***
	// TSM:    see Timer0
	// PWM1B: Pulse Width Modulator B Enable 
	// COM1B1, COM1B0: Comparator B Output Mode, Bits 1 and 0
	//  	Table 12-6. Comparator B Mode Select
	//  	COM1B1/ COM1B0 Description
	//  *	%00            Timer/Counter Comparator B disconnected from output pin OC1B.
	//  	%01            Toggle the OC1B output line.
	//  	%10            Clear the OC1B output line.
	//  	%11            Set the OC1B output line
	// FOC1B: Force Output Compare Match 1B  
	// FOC1A: Force Output Compare Match 1A
	// PSR1 : Prescaler Reset Timer/Counter1  
	// PSR0:   see Timer0
	GTCCR = (0<<TSM) | (0<<PWM1B) | (0<<COM1B1) | (0<<COM1B0) | (0<<FOC1B) | (0<<FOC1A) | (0<<PSR1) | (0<<PSR0); // complete register
	//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx only change these bits (other bits used for different function)
//	GTCCR = (GTCCR & ~((1<<PWM1B) | (1<<COM1B1) | (1<<COM1B0) | (1<<FOC1B) | (1<<FOC1A) | (1<<PSR1))) | ((1<<PWM1B) | (1<<COM1B1) | (1<<COM1B0) | (0<<FOC1B) | (0<<FOC1A) | (0<<PSR1)); // only bits used in this unit

	// *** TCNT1 – Timer/Counter1 ***
//	TCNT1 = 0;

	// *** OCR1A – Output Compare Register 1 A ***
//	OCR1A = 125;	// 1ms

	// *** OCR1B – Output Compare Register 1 B ***
//	OCR1B = 100;

	// *** OCR1C – Output Compare Register 1 C ***
//	OCR1C = 0;

	// *** TIMSK – Timer/Counter 0 Interrupt Mask Register ***
	// OCIE1B: Timer/Counter1 Output Compare Match B Interrupt Enable
	// OCIE1A: Timer/Counter1 Output Compare Match A Interrupt Enable
	// OCIE0B: see Timer0
	// OCIE0A: see Timer0
	// TOIE1:  Timer/Counter1 Overflow Interrupt Enable
	// TOIE0:  see Timer0
//	TIMSK = (0<<OCIE1A) | (0<<OCIE1B) | (0<<OCIE0A) | (0<<OCIE0B) | (0<<TOIE1) | (0<<TOIE0); // complete register
	//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx only change these bits (other bits used for different function)
	TIMSK = (TIMSK & ~((1<<OCIE1A) | (1<<OCIE1B) | (1<<TOIE1))) | ((0<<OCIE1A) | (0<<OCIE1B) | (1<<TOIE1));  // only bits used in this unit
	// Timer1 also used for simple 2ms interrupt via OCIE0B

	// *** TIFR – Timer/Counter 0 Interrupt Flag Register ***
	// OCF1B: Output Compare Flag 1 B
	// OCF1A: Output Compare Flag 1 A
	// OCF0B: see Timer0
	// OCF0A: see Timer0
	// TOV1:  Timer/Counter1 Overflow Flag
	// TOV0:  see Timer0
//	TIFR = (0<<OCF1B) | (0<<OCF1A) | (0<<OCF0B) | (0<<OCF0A) | (0<<TOV1) | (0<<TOV0); // complete register
	//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx only change these bits (other bits used for different function)
//	TIFR = (TIFR & ~((1<<OCF1B) | (1<<OCF1A) | (1<<TOV1))) | ((0<<OCF1B) | (0<<OCF1A) | (0<<TOV1)); // only bits used in this unit

	// *** PLLCSR – PLL Control and Status Register ***
	// LSM: Low Speed Mode
	// PCKE: PCK Enable
	// PLLE: PLL Enable
	// PLOCK: PLL Lock Detector
//	PLLCSR = (0<<LSM) | (0<<PCKE) | (0<<PLLE) | (0<<PLOCK);
	
	
	// *************************** 8-bit Timer/Counter1 in ATtiny15 Mode see chapter 13 / page 98 ***************************


	// *************************** Dead Time Generator ***************************
	// (datasheet page 108)

	// *** DTPS1 – Timer/Counter1 Dead Time Prescaler Register 1 ***
	// DTPS11:DTPS10: Dead Time Prescaler
	//     Table 14-1. Division factors of the Dead Time prescaler
	//  	DTPS11/DTPS10 Prescaler divides the T/C1 clock by
	//  	%00           1x (no division)
	//  	%01           2x
	//  	%10           4x
	//  	%11           8x
//	DTPS1 = (0<<DTPS11) | (0<<DTPS10);
	
	// *** DT1A – Timer/Counter1 Dead Time A ***
	// DT1AH3:DT1AH0: Dead Time Value for OC1A Output
	// DT1AL3:DT1AL0: Dead Time Value for -OC1A Output
//	DT1A = (0<<DT1AH3) | (0<<DT1AH2) | (0<<DT1AH1) | (0<<DT1AH0) | (0<<DT1AL3) | (0<<DT1AL2) | (0<<DT1AL1) | (0<<DT1AL0);
	
	// *** DT1B – Timer/Counter1 Dead Time B ***
	// DT1BH3:DT1BH0: Dead Time Value for OC1B Output
	// DT1BL3:DT1BL0: Dead Time Value for -OC1B Output
//	DT1B = (0<<DT1BH3) | (0<<DT1BH2) | (0<<DT1BH1) | (0<<DT1BH0) | (0<<DT1BL3) | (0<<DT1BL2) | (0<<DT1AB1) | (0<<DT1BL0);
	
}


/*==============================================================================
 * all project specific Timer routines:
 *==============================================================================
 */


 /*-----------------------------------------------------------------------------
 * channelcontrol
 *  @details  switch LEDs for 1 channel according pattern
 *  @param    index of channel in array
 *  @return   -
 *------------------------------------------------------------------------------
 */
void channelcontrol(uint8_t ch)
{
	if(LightChannel[ch].active)
	{
		LightChannel[ch].timecount++;	// measure time from beginning of period for resetting after deactivation
		
		// if not pulse ontime over (ontimecount<ontime)
		if (LightChannel[ch].ontimecount < pgm_read_word(&OntimeValues[LightChannel[ch].ontime]) )
		{
			// LED on
			LEDS_PORT  = LEDS_PORT_RD | (1<<LightChannel[ch].ledpin) | (1<<RC_PIN); // keep pullup at RC_PIN
			
			// count up pulse ontime
			LightChannel[ch].ontimecount++;
		}
		else
		{
			// if not last pulse (pulsecount<pulses-1)
			if (LightChannel[ch].pulsecount < (LightChannel[ch].pulses-1) )
			{
				// if not pulse offtime over (offtimepulsecount<offtimepulse)
				if (LightChannel[ch].offtimepulsecount < pgm_read_word(&OfftimeValues[LightChannel[ch].offtimepulse]))
				{
					// LED off
					LEDS_PORT  = (LEDS_PORT  & ~(1<<LightChannel[ch].ledpin)) | (1<<RC_PIN); // keep pullup at RC_PIN
					
					// count up pulse offtime
					LightChannel[ch].offtimepulsecount++;
				}
				else
				{
					LightChannel[ch].ontimecount = 0;
					LightChannel[ch].offtimepulsecount = 0;
					LightChannel[ch].pulsecount++;
				}
			}
			else
			{
				// if not offtime over (offtimecount<offtime)  ( or offtime=0 ???)
				if (LightChannel[ch].offtimecount < pgm_read_word(&OfftimeValues[LightChannel[ch].offtime]))
				{
					// LED off
					LEDS_PORT  = (LEDS_PORT  & ~(1<<LightChannel[ch].ledpin)) | (1<<RC_PIN); // keep pullup at RC_PIN
					
					// count up offtime
					LightChannel[ch].offtimecount++;
				}
				else
				{
					LightChannel[ch].ontimecount = 0;
					LightChannel[ch].offtimepulsecount = 0;
					LightChannel[ch].offtimecount = 0;
					LightChannel[ch].pulsecount = 0;
				}
			}
		}
	}
	else
	{
		// LED off
		LEDS_PORT  = (LEDS_PORT  & ~(1<<LightChannel[ch].ledpin)) | (1<<RC_PIN); // keep pullup at RC_PIN
/*
		//   if timecount < ontime*pulses + offtimepulse*pulses-1 + offtime
		if (LightChannel[ch].timecount < (pgm_read_word(&OntimeValues[LightChannel[ch].ontime])*LightChannel[ch].pulses) + (pgm_read_word(&OfftimeValues[LightChannel[ch].offtimepulse])*LightChannel[ch].pulses-1) + pgm_read_word(&OfftimeValues[LightChannel[ch].offtime]) )
		{
			LightChannel[ch].timecount++;	// measure time from beginning of period for resetting after deactivation
		}
		else
*/		{
			LightChannel[ch].timecount = 0;
			LightChannel[ch].ontimecount = 0;
			LightChannel[ch].offtimepulsecount = 0;
			LightChannel[ch].offtimecount = 0;
			LightChannel[ch].pulsecount = 0;
		}
	}
}


/*==============================================================================
 * all Timer relevant interrupt service routines:
 *==============================================================================
 */

/*------------------------------------------------------------------------------
 * Timer/Counter0 Compare Match A
 *  @details  1ms Interrupt for LED pattern changes
 *  @param    -
 *  @return   -
 *------------------------------------------------------------------------------
 */
ISR(TIM0_COMPA_vect)
{
	uint8_t	ch;
	
	sei(); // enable interrupts for RC pulse detection
	
	if(TimeCounter<65535)	// only count beginning 65 seconds
	{
		TimeCounter++;
	}

	for(ch=0;ch<NUMBER_OF_CHANNELS;ch++)	// control all channels subsequently
	{
		channelcontrol(ch);
	}
}

/*------------------------------------------------------------------------------
 * Timer/Counter0 Compare Match B
 *  @details  -
 *  @param    -
 *  @return   -
 *------------------------------------------------------------------------------
 */
ISR(TIM0_COMPB_vect)
{
	// put code here
}

/*------------------------------------------------------------------------------
 * Timer/Counter0 Overflow
 *  @details  -
 *  @param    -
 *  @return   -
 *------------------------------------------------------------------------------
 */
ISR(TIM0_OVF_vect)
{
	// put code here
}

/*------------------------------------------------------------------------------
 * Timer/Counter1 Compare Match 1A
 *  @details  -
 *  @param    -
 *  @return   -
 *------------------------------------------------------------------------------
 */
ISR(TIM1_COMPA_vect)
{
	// put code here
}

/*------------------------------------------------------------------------------
 * Timer/Counter1 Overflow
 *  @details  detect missing pulse / missing RC signal
 *  @param    -
 *  @return   -
 *------------------------------------------------------------------------------
 */
ISR(TIM1_OVF_vect)
{
	RCpulse_timeout=true;
	
	// check for pressed button instead of RC signal:
	if (RC_PORT_RD & (1<<RC_PIN))  // high = button not pressed
	{
		if (button_release_count<255)	// prevent overflow
		{
			button_release_count++;
		}
		button_pressed_count = 0;
		if ((button_was_read) && (button_release_count==24)) // released longer than 100ms
		{
			button_state = BUTTON_RELEASED;
		}
	}
	else  // low = button pressed
	{
		if (button_pressed_count<255)	// prevent overflow
		{
			button_pressed_count++;
		}
		button_release_count = 0;
		if ((button_was_read) && (button_pressed_count==24)) // pressed longer than 100ms
		{
			button_state = BUTTON_PRESSED;
			button_was_read = false;	// new button pressing
		}
	}
}

/*------------------------------------------------------------------------------
 * Timer/Counter1 Compare Match B
 *  @details  -
 *  @param    -
 *  @return   -
 *------------------------------------------------------------------------------
 */
ISR(TIM1_COMPB_vect)
{
	// put code here
}

