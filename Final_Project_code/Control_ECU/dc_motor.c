#include "dc_motor.h"
#include "gpio.h"


/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/


void DcMotor_Init(void)		// to initialize the DC motor
{
	GPIO_setupPinDirection(MOTOR_IN1_PORT, MOTOR_IN1_PIN, MOTOR_OUTPUT_PIN);
	GPIO_setupPinDirection(MOTOR_IN2_PORT, MOTOR_IN2_PIN, MOTOR_OUTPUT_PIN);
	GPIO_setupPinDirection(MOTOR_ENABLE_1_PORT, MOTOR_ENABLE_1_PIN, MOTOR_OUTPUT_PIN);

	/* ENABLE THE MOTOR FIRST BY ENABLE = 1*/
	GPIO_writePin(MOTOR_ENABLE_1_PORT, MOTOR_ENABLE_1_PIN,MOTOR_ON);
	/*TO STOP THE MOTOR INI1 = 0 INT2 = 0
	 * FOR CLOCKWISE INT1 = 0 INT2 = 1
	 * FOR ANTI-CLOCKWISE INT1 = 1 INT2 = 0

	WE NEED TO STOP THE MOTOR FIRSTLY
	 */

	GPIO_writePin(MOTOR_IN1_PORT, MOTOR_IN1_PIN,MOTOR_OFF);
	GPIO_writePin(MOTOR_IN2_PORT, MOTOR_IN2_PIN,MOTOR_OFF);

}


// THE SPEED HERE MUST BE A PERCENTAGE OF MOXIMUM AND THE STATE IS A ENUM OF DIRECTION OR STOP

void DcMotor_Rotate(DcMotor_State state)	// To rotate the DC motor in which direction
{
	switch (state)
	{
	case STOP:
		GPIO_writePin(MOTOR_IN1_PORT, MOTOR_IN1_PIN,MOTOR_OFF);
		GPIO_writePin(MOTOR_IN2_PORT, MOTOR_IN2_PIN,MOTOR_OFF);
		break;
	case CLOCKWISE:

		GPIO_writePin(MOTOR_IN1_PORT, MOTOR_IN1_PIN,MOTOR_OFF);
		GPIO_writePin(MOTOR_IN2_PORT, MOTOR_IN2_PIN,MOTOR_ON);
		break;
	case ANTI_CLOCKWISE:
		GPIO_writePin(MOTOR_IN1_PORT, MOTOR_IN1_PIN,MOTOR_ON);
		GPIO_writePin(MOTOR_IN2_PORT, MOTOR_IN2_PIN,MOTOR_OFF);
		break;

	}


}

