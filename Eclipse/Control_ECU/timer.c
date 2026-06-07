 /******************************************************************************
 *
 * Module: Timer
 *
 * File Name: timer.c
 *
 * Description: Generic source file for the Timer AVR driver
 *
 * Author: Ziyad Ehab
 *
 *******************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"

/* Callback function pointers for each timer */
static volatile void (*g_Timer0_Callback)(void) = NULL;
static volatile void (*g_Timer1_Callback)(void) = NULL;
static volatile void (*g_Timer2_Callback)(void) = NULL;

/* Interrupt Service Routines (ISRs) */

/* Timer0 Overflow Interrupt */
ISR(TIMER0_OVF_vect) {
    if (g_Timer0_Callback != NULL) {
        (*g_Timer0_Callback)();
    }
}

/* Timer0 Compare Match Interrupt */
ISR(TIMER0_COMP_vect) {
    if (g_Timer0_Callback != NULL) {
        (*g_Timer0_Callback)();
    }
}

/* Timer1 Overflow Interrupt */
ISR(TIMER1_OVF_vect) {
    if (g_Timer1_Callback != NULL) {
        (*g_Timer1_Callback)();
    }
}

/* Timer1 Compare Match A Interrupt */
ISR(TIMER1_COMPA_vect) {
    if (g_Timer1_Callback != NULL) {
        (*g_Timer1_Callback)();
    }
}

/* Timer2 Overflow Interrupt */
ISR(TIMER2_OVF_vect) {
    if (g_Timer2_Callback != NULL) {
        (*g_Timer2_Callback)();
    }
}

/* Timer2 Compare Match Interrupt */
ISR(TIMER2_COMP_vect) {
    if (g_Timer2_Callback != NULL) {
        (*g_Timer2_Callback)();
    }
}

/* Initialize the timer with the provided configuration */
void Timer_init(const Timer_ConfigType * Config_Ptr) {
    switch (Config_Ptr->timer_ID) {
        case TIMER0:
            /* Configure Timer0 (8-bit) */
            if (Config_Ptr->timer_mode == NORMAL_MODE) {
                /* Normal mode: WGM00=0, WGM01=0 */
                TCCR0 = (0 << WGM00) | (0 << WGM01);
                /* Enable overflow interrupt */
                TIMSK |= (1 << TOIE0);
            } else if (Config_Ptr->timer_mode == COMPARE_MODE) {
                /* CTC mode: WGM00=0, WGM01=1 */
                TCCR0 = (0 << WGM00) | (1 << WGM01);
                /* Set compare value (8-bit) */
                OCR0 = (uint8_t)Config_Ptr->timer_compare_MatchValue;
                /* Enable compare match interrupt */
                TIMSK |= (1 << OCIE0);
            }
            /* Set initial value (8-bit) */
            TCNT0 = (uint8_t)Config_Ptr->timer_InitialValue;
            /* Set clock source */
            TCCR0 = (TCCR0 & 0xF8) | (Config_Ptr->timer_clock & 0x07);
            break;

        case TIMER1:
            /* Configure Timer1 (16-bit) */
            TCCR1A = 0; /* Clear TCCR1A (COM bits = 0, WGM10=0, WGM11=0) */
            if (Config_Ptr->timer_mode == NORMAL_MODE) {
                /* Normal mode: WGM13:0 = 0000 */
                TCCR1B = (0 << WGM12) | (0 << WGM13);
                /* Enable overflow interrupt */
                TIMSK |= (1 << TOIE1);
            } else if (Config_Ptr->timer_mode == COMPARE_MODE) {
                /* CTC mode with OCR1A: WGM13:0 = 0100 */
                TCCR1B = (1 << WGM12) | (0 << WGM13);
                /* Set compare value (16-bit) */
                OCR1AH = (Config_Ptr->timer_compare_MatchValue >> 8);
                OCR1AL = (uint8_t)Config_Ptr->timer_compare_MatchValue;
                /* Enable compare match A interrupt */
                TIMSK |= (1 << OCIE1A);
            }
            /* Set initial value (16-bit) */
            TCNT1H = (Config_Ptr->timer_InitialValue >> 8);
            TCNT1L = (uint8_t)Config_Ptr->timer_InitialValue;
            /* Set clock source */
            TCCR1B = (TCCR1B & 0xF8) | (Config_Ptr->timer_clock & 0x07);
            break;

        case TIMER2:
            /* Configure Timer2 (8-bit) */
            if (Config_Ptr->timer_mode == NORMAL_MODE) {
                /* Normal mode: WGM20=0, WGM21=0 */
                TCCR2 = (0 << WGM20) | (0 << WGM21);
                /* Enable overflow interrupt */
                TIMSK |= (1 << TOIE2);
            } else if (Config_Ptr->timer_mode == COMPARE_MODE) {
                /* CTC mode: WGM20=0, WGM21=1 */
                TCCR2 = (0 << WGM20) | (1 << WGM21);
                /* Set compare value (8-bit) */
                OCR2 = (uint8_t)Config_Ptr->timer_compare_MatchValue;
                /* Enable compare match interrupt */
                TIMSK |= (1 << OCIE2);
            }
            /* Set initial value (8-bit) */
            TCNT2 = (uint8_t)Config_Ptr->timer_InitialValue;
            /* Set clock source */
            TCCR2 = (TCCR2 & 0xF8) | (Config_Ptr->timer_clock & 0x07);
            break;
    }
}

/* Disable the specified timer */
void Timer_delnit(Timer_ID_Type timer_type) {
    switch (timer_type) {
        case TIMER0:
            /* Stop Timer0 by clearing clock bits */
            TCCR0 &= ~0x07;
            /* Disable interrupts */
            TIMSK &= ~((1 << TOIE0) | (1 << OCIE0));
            break;
        case TIMER1:
            /* Stop Timer1 by clearing clock bits */
            TCCR1B &= ~0x07;
            /* Disable interrupts */
            TIMSK &= ~((1 << TOIE1) | (1 << OCIE1A));
            break;
        case TIMER2:
            /* Stop Timer2 by clearing clock bits */
            TCCR2 &= ~0x07;
            /* Disable interrupts */
            TIMSK &= ~((1 << TOIE2) | (1 << OCIE2));
            break;
    }
}

/* Set the callback function for the specified timer */
void Timer_setCallback(void(*a_ptr)(void), Timer_ID_Type a_timer_ID) {
    switch (a_timer_ID) {
        case TIMER0:
            g_Timer0_Callback = a_ptr;
            break;
        case TIMER1:
            g_Timer1_Callback = a_ptr;
            break;
        case TIMER2:
            g_Timer2_Callback = a_ptr;
            break;
    }
}
