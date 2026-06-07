 /******************************************************************************
 *
 * Module: PIR Sensor
 *
 * File Name: PIR.c
 *
 * Description: Source file for the PIR Sensor driver
 *
 * Author:	Zeyad Ehab
 *
 *******************************************************************************/
#include "PIR.h"

void PIR_init(void)					////Function to initialize the PIR driver
{
	GPIO_setupPinDirection(PIR_INPUT_PORT, PIR_INPUT_PIN, PIN_INPUT);	//Initialize the Pin direction
}

uint8 PIR_getState(void)			//Function to return PIR State
{
	uint8 motion = 0;
	motion = GPIO_readPin(PIR_INPUT_PORT,PIR_INPUT_PIN);		//Read Pin value
	return motion;
}
