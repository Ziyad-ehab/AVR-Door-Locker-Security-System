 /******************************************************************************
 *
 * Module: UART
 *
 * File Name: uart.h
 *
 * Description: Header file for the UART AVR driver
 *
 * Author:	Zeyad Ehab
 *
 *******************************************************************************/

#ifndef UART_H_
#define UART_H_
#include "std_types.h"
#include "gpio.h"


#define UART_TX_PORT_ID		PORTD_ID
#define UART_TX_PIN_ID		PIN1_ID
#define UART_RX_PORT_ID		PORTD_ID
#define UART_RX_PIN_ID		PIN0_ID


/*  Define new data types for the UAART structure configurations */
typedef enum
{
	FIVE_BITS,
	SIX_BITS,
	SEVEN_BITS,
	EIGHT_BITS,
	NINE_BITS
}UART_BitDataType ;


typedef enum
{
	 DISABLE=0b00,
	 EVEN_PARITY=0b10,
	 ODD_PARITY=0b11
}UART_ParityType;


typedef enum
{
	 ONE_BIT = 0,
	 TWO_BIT = 1
}UART_StopBitType;


typedef uint32 UART_BaudRateType ;

/* UART Configuration structure to be passed in the main function*/
typedef struct {
	UART_BitDataType bit_data;
	UART_ParityType parity;
	UART_StopBitType stop_bit;
	UART_BaudRateType baud_rate;
} UART_ConfigType;

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/*
 * Description :
 * Functional responsible for Initialize the UART device by:
 * 1. Setup the Frame format like number of data bits, parity bit type and number of stop bits.
 * 2. Enable the UART.
 * 3. Setup the UART baud rate.
 */
void UART_init(const UART_ConfigType * Config_Ptr);

/*
 * Description :
 * Functional responsible for send byte to another UART device.
 */
void UART_sendByte(const uint8 data);

/*
 * Description :
 * Functional responsible for receive byte from another UART device.
 */
uint8 UART_recieveByte(void);

/*
 * Description :
 * Send the required string through UART to the other UART device.
 */
void UART_sendString(const uint8 *Str);

/*
 * Description :
 * Receive the required string until the '#' symbol through UART from the other UART device.
 */
void UART_receiveString(uint8 *Str); // Receive until #

#endif /* UART_H_ */
