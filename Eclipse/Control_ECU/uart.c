 /******************************************************************************
 *
 * Module: UART
 *
 * File Name: uart.c
 *
 * Description: Source file for the UART AVR driver
 *
 * Author: Zeyad Ehab
 *
 *******************************************************************************/

#include "uart.h"
#include "avr/io.h" /* To use the UART Registers */
#include "common_macros.h" /* To use the macros like SET_BIT */

/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/

/*
 * Description :
 * Functional responsible for Initialize the UART device by:
 * 1. Setup the Frame format like number of data bits, parity bit type and number of stop bits.
 * 2. Enable the UART.
 * 3. Setup the UART baud rate.
 */


/* UART Configuration structure to be passed in the main function*/
/*
typedef struct {
	UART_BitDataType bit_data;
	UART_ParityType parity;
	UART_StopBitType stop_bit;
	UART_BaudRateType baud_rate;
} UART_ConfigType;
*/


void UART_init(const UART_ConfigType * Config_Ptr)
{
	// Configure TX and RX to OUTPUT AND INPUT PINS
	GPIO_setupPinDirection(UART_TX_PORT_ID, UART_TX_PIN_ID, PIN_OUTPUT);		// CONFIGURE TX PIN AS OUTPUT
	GPIO_setupPinDirection(UART_RX_PORT_ID, UART_RX_PIN_ID, PIN_INPUT);			//CONFIGURE RX PIN AS INPUT


	uint16 ubrr_value = 0;

	/* U2X = 1 for double transmission speed */
	UCSRA = (1<<U2X);



	/************************** UCSRB Description **************************
	 * RXCIE = 0 Disable USART RX Complete Interrupt Enable
	 * TXCIE = 0 Disable USART Tx Complete Interrupt Enable
	 * UDRIE = 0 Disable USART Data Register Empty Interrupt Enable
	 * RXEN  = 1 Receiver Enable
	 * RXEN  = 1 Transmitter Enable
	 * UCSZ2 = 0 For 8-bit data mode
	 * RXB8 & TXB8 not used for 8-bit data mode
	 ***********************************************************************/ 
	UCSRB = (1<<RXEN) | (1<<TXEN);
	


	/************************** UCSRC Description **************************
	 * URSEL   = 1 The URSEL must be one when writing the UCSRC
	 * UMSEL   = 0 Asynchronous Operation
	 * UPM1:0  = 00 Disable parity bit  10 for EVEN PARITY and 11 for ODD PARITY
	 * USBS    = 0 One stop bit
	 * UCSZ1:0 = 11 For 8-bit data mode
	 * UCPOL   = 0 Used with the Synchronous operation only
	 ***********************************************************************/ 	
	UCSRC = (1<<URSEL);

	switch(Config_Ptr->parity)
	{
	case DISABLE:			//UPM1:0 = 00
		break;
	case EVEN_PARITY:		//UPM1:0 = 10
		UCSRC |= (0x20);
		break;
	case ODD_PARITY:
		UCSRC |= (0x30);	//UPM1:0 = 11
		break;
	}

	switch(Config_Ptr->stop_bit)
	{
	case ONE_BIT:		//USBS = 0
		break;
	case TWO_BIT:		//USBS = 1
		UCSRC |= (0x08);
		break;
	}

	/* UCSZ REGISTER FOR DATA MODE*/
	switch(Config_Ptr->bit_data)
	{
	case FIVE_BITS:			//UCSZ0:1 = 00
		break;
	case SIX_BITS:
		UCSRC |= (0x02); 	// UCSZ0:1 = 01
		break;
	case SEVEN_BITS:
		UCSRC |= (0x04);	//UCSZ0:1 = 10
		break;
	case EIGHT_BITS:
		UCSRC |= (0x06);	//UCSZ0:1 = 11
		break;
	case NINE_BITS:
		UCSRC |= (0x06);	//UCSZ0:1 = 11
		UCSRB |= (0X04);	//UCSZ2 = 1
		break;
	}

	
	/* Calculate the UBRR register value */
	ubrr_value = (uint16)(((F_CPU / ((Config_Ptr->baud_rate) * 8UL))) - 1);

	/* First 8 bits from the BAUD_PRESCALE inside UBRRL and last 4 bits in UBRRH*/
	UBRRH = ubrr_value>>8;
	UBRRL = ubrr_value;
}

/*
 * Description :
 * Functional responsible for send byte to another UART device.
 */
void UART_sendByte(const uint8 data)
{
	/*
	 * UDRE flag is set when the Tx buffer (UDR) is empty and ready for
	 * transmitting a new byte so wait until this flag is set to one
	 */
	while(BIT_IS_CLEAR(UCSRA,UDRE)){}

	/*
	 * Put the required data in the UDR register and it also clear the UDRE flag as
	 * the UDR register is not empty now
	 */
	UDR = data;

	/************************* Another Method *************************
	UDR = data;
	while(BIT_IS_CLEAR(UCSRA,TXC)){} // Wait until the transmission is complete TXC = 1
	SET_BIT(UCSRA,TXC); // Clear the TXC flag
	*******************************************************************/
}

/*
 * Description :
 * Functional responsible for receive byte from another UART device.
 */
uint8 UART_recieveByte(void)
{
	/* RXC flag is set when the UART receive data so wait until this flag is set to one */
	while(BIT_IS_CLEAR(UCSRA,RXC)){}

	/*
	 * Read the received data from the Rx buffer (UDR)
	 * The RXC flag will be cleared after read the data
	 */
    return UDR;		
}

/*
 * Description :
 * Send the required string through UART to the other UART device.
 */
void UART_sendString(const uint8 *Str)
{
	uint8 i = 0;

	/* Send the whole string */
	while(Str[i] != '\0')
	{
		UART_sendByte(Str[i]);
		i++;
	}
	/************************* Another Method *************************
	while(*Str != '\0')
	{
		UART_sendByte(*Str);
		Str++;
	}		
	*******************************************************************/
}

/*
 * Description :
 * Receive the required string until the '#' symbol through UART from the other UART device.
 */
void UART_receiveString(uint8 *Str)
{
	uint8 i = 0;

	/* Receive the first byte */
	Str[i] = UART_recieveByte();

	/* Receive the whole string until the '#' */
	while(Str[i] != '#')
	{
		i++;
		Str[i] = UART_recieveByte();
	}

	/* After receiving the whole string plus the '#', replace the '#' with '\0' */
	Str[i] = '\0';
}
