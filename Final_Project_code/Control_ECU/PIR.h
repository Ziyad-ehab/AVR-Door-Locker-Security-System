 /******************************************************************************
 *
 * Module: PIR Sensor
 *
 * File Name: PIR.h
 *
 * Description: Header file for the PIR Sensor driver
 *
 * Author:	Zeyad Ehab
 *
 *******************************************************************************/

#ifndef PIR_H_
#define PIR_H_

#include "std_types.h"
#include "gpio.h"

/*******************************************************************************
 *                                Definitions                                  *
 ******************************************************************************/
//DEFINE PIR SENSOR CONFIGURATION
#define PIR_INPUT_PORT		PORTC_ID
#define PIR_INPUT_PIN		PIN2_ID

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/
void PIR_init(void);			//Function to initialize the PIR driver
uint8 PIR_getState(void);		//Function to return PIR State

#endif /* PIR_H_ */
