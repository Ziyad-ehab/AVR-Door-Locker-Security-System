 /******************************************************************************
 *
 * Module: PWM source file
 *
 * File Name: PWM.c
 *
 * Description: PWM Source file for AVR
 *
 * Author: Zeyad Ehab
 *
 *******************************************************************************/

#include "PWM.h"
#include <avr/io.h>
#include "gpio.h"

/*
 * Description:
 * Timer0 will be used with pre-scaler F_CPU/64
 * F_PWM=(F_CPU)/(256*N) = (8*10^6)/(256*64) = 488.2Hz
 * Duty Cycle can be changed by updating the value
 * in The Compare Register (input the duty cycle by a percentage)
 */

void Timer0_PWM_Init(unsigned char set_duty_cycle)
{
	//configure OC0 as the output pin.
	GPIO_setupPinDirection(OC0_PORT_ID, OC0_PIN_ID, PIN_OUTPUT);
	TCNT0 = 0; // Set Timer Initial Value to 0
	/* equation to input the duty cycle as a percentage of the max*/
	set_duty_cycle = (set_duty_cycle * MAX_DUTY_CYCLE) /100;
	OCR0  = set_duty_cycle; //Set Compare value

	// Configure PB3/OC0 as output pin --> pin where the PWM signal is generated from MC

	/* Configure timer control register
	 * 1. Fast PWM mode FOC0=0
	 * 2. Fast PWM Mode WGM01=1 & WGM00=1
	 * 3. Clear OC0 when match occurs (non inverted mode) COM00=0 & COM01=1
	 * 4. clock = F_CPU/64 CS00=1 CS01=1 CS02=0
	 */
	TCCR0 = (1<<WGM00) | (1<<WGM01) | (1<<COM01) | (1<<CS01) |(1<<CS00);
}

