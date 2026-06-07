 /******************************************************************************
 *
 * Module: PWM header file
 *
 * File Name: PWM.h
 *
 * Description: PWM header file for AVR
 *
 * Author: Zeyad Ehab
 *
 *******************************************************************************/

#ifndef PWM_H_
#define PWM_H_
#include "std_types.h"


/*******************************************************************************
 *                                Definitions                                  *
 ********************************	***********************************************/
#define MAX_DUTY_CYCLE 255
#define OC0_PORT_ID PORTB_ID
#define OC0_PIN_ID PIN3_ID

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/
void Timer0_PWM_Init(uint8 set_duty_cycle);
#endif /* PWM_H_ */
