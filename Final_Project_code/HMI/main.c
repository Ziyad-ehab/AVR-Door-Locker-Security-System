#include "lcd.h"
#include "keypad.h"
#include "uart.h"
#include "timer.h"
#include "interrupt.h"
#include <avr/io.h>
#include <util/delay.h>

#define PASSWORD_LENGTH 5
#define ENTER_KEY '='
#define OPEN_DOOR_KEY '*'
#define CHANGE_PASSWORD_KEY '%'

typedef enum {									// define thew state the controller in in now
    INIT,
    SET_PASSWORD,
    MAIN_OPTIONS,
    ENTER_PASSWORD,
    OPEN_DOOR,
    CHANGE_PASSWORD,
    LOCK_SYSTEM
} State;

State currentState = INIT;						// initialize the state into initial state
uint8 password[PASSWORD_LENGTH];				// set the arrays size to the standard password size
uint8 confirmPassword[PASSWORD_LENGTH];			// array used to confirm the password with the control mcu
uint8 enteredPassword[PASSWORD_LENGTH];			// send the entered array to compare it to the control_mcu
uint8 savedPassword[PASSWORD_LENGTH];			// to display the sent password after confiming it
uint8 selectedOption;							// the selected option form the user
uint8 attempts = 0;								// for wrong attempts only
//timer top was 31249
// need to modify the ticks to 60 tick for buzzer
UART_ConfigType uartConfig = {EIGHT_BITS, DISABLE, ONE_BIT, 9600};		//UART configurations
Timer_ConfigType timerConfig = {0, 46874, TIMER1, F_CPU_CLOCK_1024, COMPARE_MODE};	//	TIMER 1 configurations

void readPassword(uint8* pw) {					// function to read the password from the user
    uint8 i = 0;
    LCD_moveCursor(1, 0);

    while(i<PASSWORD_LENGTH)
    {
        uint8 key = KEYPAD_getPressedKey();
        if (key >= 0 && key <= 9) {			// ensure only numbers are pressed by the user
            pw[i] = key;
            LCD_displayCharacter('*');			// display * instead of the actual number
            i++;
        }
        _delay_ms(300); // Debounce delay		// CHECK IF REQUIRED
    }
    while (KEYPAD_getPressedKey() != ENTER_KEY);  // Dont leave this function until 5 numbers are pressed
}

void sendCommand(const uint8 cmd) {				// function to send the current command to the Control MCU
	UART_sendByte(cmd);
}

void sendPassword(const uint8* pw) {			// Send the entered password by the user using UART but the HMI_MCU
    for (uint8 i = 0; i < PASSWORD_LENGTH; i++) {
        UART_sendByte(pw[i]);
    }
}

uint8 receiveResponse() {						// Receives the UART receive bute form the control_MCU
    return UART_recieveByte();
}

void displayMessage(const char* msg) {			// General way to display a message and clearing the previous one
    LCD_clearScreen();
    LCD_displayString(msg);
}

void handleSetPassword() {
    displayMessage("Enter Password:");
    readPassword(password);					// name of the array is its address
    displayMessage("Confirm Password:");
    readPassword(confirmPassword);			// name of the array is its address
    LCD_clearScreen();

    /*			USED the next line to see the save passwords			*/
/*
    int i=0;
           for(i = 0;i<5;i++)
           {
           LCD_moveCursor(0, i);
           LCD_intgerToString(password[i]);
           LCD_moveCursor(1, i);
           LCD_intgerToString(confirmPassword[i]);
           }


    _delay_ms(5000);

*/

    sendCommand((uint8)'S');
    sendPassword(password);
    sendPassword(confirmPassword);
    uint8 response = receiveResponse();
    if (response == 'S') {
		LCD_displayStringRowColumn(0,0,"Saved Pass = ");

    	for (uint8 i = 0; i < PASSWORD_LENGTH; i++)
    	    	{
    		savedPassword[i] = UART_recieveByte();
    		LCD_displayStringRowColumn(1,i,savedPassword[i]);
    	    	}
    	_delay_ms(4000);

        currentState = MAIN_OPTIONS;
    } else {
        displayMessage("Mismatch! Try again");
        _delay_ms(1000);
    }
}

void handleMainOptions() {
	LCD_displayStringRowColumn(0,0,"* : Open Door");
	LCD_displayStringRowColumn(1,0,"% : Change PW");

    uint8 key;
    do {
        key = KEYPAD_getPressedKey();
        _delay_ms(300);
    } while (key != OPEN_DOOR_KEY && key != CHANGE_PASSWORD_KEY);
    selectedOption = (key == OPEN_DOOR_KEY) ? 'O' : 'C';
    currentState = ENTER_PASSWORD;
}

void handleEnterPassword() {
    displayMessage("Enter Password:");
    readPassword(enteredPassword);
    sendCommand((uint8)'V');
    sendPassword(enteredPassword);
    uint8 response = receiveResponse();
    if (response == 'M') {
        attempts = 0;
        currentState = (selectedOption == 'O') ? OPEN_DOOR : CHANGE_PASSWORD;
    } else {
        attempts++;
        if (attempts < 3) {
            displayMessage("Wrong PW! Try again");
            _delay_ms(1000);
        } else {
            sendCommand((uint8)'L');
            currentState = LOCK_SYSTEM;
        }
    }
}

void handleOpenDoor() {
    sendCommand((uint8)'O');
    while (1) {
        uint8 status = receiveResponse();
        if (status == 'U') displayMessage("Door is Unlocking");
        else if (status == 'W') displayMessage("Wait for people");
        else if (status == 'L') displayMessage("Door is Locking");
        else if (status == 'D') {
            currentState = MAIN_OPTIONS;
            break;
        }
    }
}

volatile uint8 lockDone = 0;
void lockCallback()
{ lockDone = 1; }

void handleLockSystem()
{
    displayMessage("Error: Locked");
    lockDone = 0;
    Timer_setCallback(lockCallback, TIMER1);
    Timer_init(&timerConfig); // Start 1-min timer (60 interrupts)
    while (!lockDone); // Ignore inputs during lock
    currentState = MAIN_OPTIONS;
}

ISR(TIMER1_COMPA_vect)
{
    static uint8 seconds = 0;
    if (++seconds >= 60) {
        seconds = 0;
        lockDone = 1;
        Timer_delnit(TIMER1);
    }
}

int main(void)
{
    LCD_init();
    UART_init(&uartConfig);
    Enable_Global_Interrupt();

    while (1) {
        switch (currentState) {
            case INIT:
                sendCommand((uint8)'C');
                if (receiveResponse() == 'N')
                {
                	currentState = SET_PASSWORD;
                }

                else currentState = MAIN_OPTIONS;
                break;
            case SET_PASSWORD:
                handleSetPassword();
                break;
            case MAIN_OPTIONS:
                handleMainOptions();
                break;
            case ENTER_PASSWORD:
                handleEnterPassword();
                break;
            case OPEN_DOOR:
                handleOpenDoor();
                break;
            case CHANGE_PASSWORD:
                handleSetPassword();
                break;
            case LOCK_SYSTEM:
                handleLockSystem();
                break;
        }
    }

    return 0;
}
