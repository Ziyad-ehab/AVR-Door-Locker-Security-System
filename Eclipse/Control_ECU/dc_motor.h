 /******************************************************************************
 *
 * Module: DC motor Header file
 *
 * File Name: dc_motor.h
 *
 * Description: DC motor header file
 *
 * Author: Zeyad Ehab
 *
 *******************************************************************************/
#ifndef DC_MOTOR_H_
#define DC_MOTOR_H_
#include "std_types.h"

/*******************************************************************************
 *                                Definitions                                  *
 ********************************	***********************************************/
#define MOTOR_IN1_PORT			PORTD_ID
#define MOTOR_IN1_PIN			PIN6_ID

#define MOTOR_IN2_PORT			PORTD_ID
#define MOTOR_IN2_PIN			PIN7_ID

#define MOTOR_ENABLE_1_PORT		PORTB_ID
#define MOTOR_ENABLE_1_PIN		PIN3_ID

#define MOTOR_OUTPUT_PIN		PIN_OUTPUT

#define MOTOR_ON				LOGIC_HIGH
#define MOTOR_OFF				LOGIC_LOW

/*******************************************************************************
 *                               Types Declaration                             *
 *******************************************************************************/
typedef enum
{
	STOP , CLOCKWISE , ANTI_CLOCKWISE
}DcMotor_State;


/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

void DcMotor_Init(void);		// to initialize the DC motor

void DcMotor_Rotate(DcMotor_State state);	// To rotate the DC motor according to the speed required in which direction


#endif /* DC_MOTOR_H_ */
