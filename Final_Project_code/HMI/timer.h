 /******************************************************************************
 *
 * Module: TIMER
 *
 * File Name: Timer.h
 *
 * Description: Header file for the AVR Timer driver
 *
 * Author: Zeyad Ehab
 *
 *******************************************************************************/
#ifndef TIMER_H
#define TIMER_H

#include "std_types.h"

/* Enum for timer IDs */
typedef enum {
	TIMER0,
    TIMER1,
    TIMER2
} Timer_ID_Type;

/* Enum for clock sources/prescalers */
typedef enum {
    NO_CLOCK = 0b000,
    F_CPU_CLOCK = 0b001,
    F_CPU_CLOCK_8 = 0b010,
    F_CPU_CLOCK_64 = 0b011,
    F_CPU_CLOCK_256 = 0b100,
    F_CPU_CLOCK_1024 = 0b101,
    EXTERNAL_CLOCK_FALLING = 0b110,
    EXTERNAL_CLOCK_RISING = 0b111
} Timer_ClockType;

/* Enum for timer modes */
typedef enum {
    NORMAL_MODE,
    COMPARE_MODE
} Timer_ModeType;

/* Configuration structure for the timer */
typedef struct {
    uint16 timer_InitialValue;         /* Initial value for the timer counter */
    uint16 timer_compare_MatchValue;   /* Compare match value (used in compare mode only) */
    Timer_ID_Type timer_ID;            /* Timer to configure (Timer0, Timer1, Timer2) */
    Timer_ClockType timer_clock;       /* Clock source/prescaler */
    Timer_ModeType timer_mode;         /* Operation mode (normal or compare) */
} Timer_ConfigType;

/* Function prototypes */
void Timer_init(const Timer_ConfigType * Config_Ptr);
void Timer_delnit(Timer_ID_Type timer_type);
void Timer_setCallback(void(*a_ptr)(void), Timer_ID_Type a_timer_ID);

#endif /* TIMER_H */
