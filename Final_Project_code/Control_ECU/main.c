#include "external_eeprom.h"
#include "pir.h"
#include "dc_motor.h"
#include "buzzer.h"
#include "uart.h"
#include "timer.h"
#include "interrupt.h"
#include "twi.h" // Added for TWI/I2C
#include <avr/io.h>
#include <util/delay.h> // Added for _delay_ms

#define PASSWORD_LENGTH 5
#define PW_FLAG_ADDRESS 0x05
#define PW_SET_FLAG 0x55

uint8 storedPassword[PASSWORD_LENGTH];
uint8 isPasswordSet = 0;
uint8 g_ticks = 0;

UART_ConfigType uartConfig = {EIGHT_BITS, DISABLE, ONE_BIT, 9600}; // UART configurations
/*		To get the timer to count every 6 seconds		*/
/*	8 Mhz F_CPU Freq and F_PWM = 1/6
 * Top = 46874		PRE_SCALAR = 1024
 */
Timer_ConfigType timerConfig = {0, 46874, TIMER1, F_CPU_CLOCK_1024, COMPARE_MODE}; // TIMER 1 configurations
TWI_ConfigType twiConfig = {I2C_Master_Address,I2C_Bit_Rate};						//I2C configurations


void loadPassword() {
    uint8 tempData;
    if (EEPROM_readByte(PW_FLAG_ADDRESS, &tempData) == 0 && tempData == PW_SET_FLAG) {
        for (uint8 i = 0; i < PASSWORD_LENGTH; i++) {
            if (EEPROM_readByte(i, &tempData) == 0) {
                storedPassword[i] = tempData;
            }
        }
        isPasswordSet = 1;
    }
}

void savePassword(const uint8* pw) {
    for (uint8 i = 0; i < PASSWORD_LENGTH; i++) {
        EEPROM_writeByte(i, pw[i]);
        _delay_ms(10); // Added delay for EEPROM write cycle
    }
    EEPROM_writeByte(PW_FLAG_ADDRESS, PW_SET_FLAG);
    _delay_ms(10); // Added delay for EEPROM write cycle
    isPasswordSet = 1;
}

uint8 comparePasswords(const uint8* pw1, const uint8* pw2) {
    for (uint8 i = 0; i < PASSWORD_LENGTH; i++) {
        if (pw1[i] != pw2[i]) return 0;
    }
    return 1;
}

void handleSetPassword() {
    uint8 pw1[PASSWORD_LENGTH], pw2[PASSWORD_LENGTH];
    for (uint8 i = 0; i < PASSWORD_LENGTH; i++)
    	{
    	pw1[i] = UART_recieveByte();
    	}
    for (uint8 i = 0; i < PASSWORD_LENGTH; i++)
    	{
    	pw2[i] = UART_recieveByte();
    	}
    if (comparePasswords(pw1, pw2)) {
        savePassword(pw1);
        UART_sendByte('S');
    } else {
        UART_sendByte('N');
    }
}

void handleVerifyPassword() {
    uint8 pw[PASSWORD_LENGTH];
    for (uint8 i = 0; i < PASSWORD_LENGTH; i++)
    	{
    	pw[i] = UART_recieveByte();
    	}
    if (isPasswordSet && comparePasswords(pw, storedPassword)) {
        UART_sendByte('M');
    } else {
        UART_sendByte('N');
    }
}

volatile uint8 motorDone = 0;
void motorCallback()
{ g_ticks++;
	if(g_ticks == 10)
	{
		motorDone = 1;
		g_ticks = 0;
	}
}

void handleOpenDoor() {
    // Unlock: CW for 15s
    DcMotor_Rotate(CLOCKWISE);
    UART_sendByte('U');
    motorDone = 0;
    Timer_setCallback(motorCallback, TIMER1);
    Timer_init(&timerConfig); // Start 15s timer
    while (!motorDone);
    DcMotor_Rotate(STOP);

    // Wait for PIR
    UART_sendByte('W');
    while (PIR_getState() == 1); // Motion detected

    // Lock: ACW for 15s
    DcMotor_Rotate(ANTI_CLOCKWISE);
    UART_sendByte('L');
    motorDone = 0;
    Timer_init(&timerConfig);
    while (!motorDone);
    DcMotor_Rotate(STOP);
    UART_sendByte('D');
}

void handleLockSystem() {
    Buzzer_on();
    motorDone = 0;
    Timer_setCallback(motorCallback, TIMER1);
    Timer_init(&timerConfig); // 1-min timer
    while (!motorDone);
    Buzzer_off();
}

ISR(TIMER1_COMPA_vect) {
    static uint8 seconds = 0;
    if (++seconds >= (TCNT1 == 0 ? 15 : 60)) { // 15s for motor, 60s for buzzer
        seconds = 0;
        motorDone = 1;
        Timer_delnit(TIMER1);
    }
}

int main(void) {
	TWI_init(&twiConfig);
    PIR_init();
    DcMotor_Init();
    Buzzer_init();
    UART_init(&uartConfig);
    Timer_init(&timerConfig);
    loadPassword();
    Enable_Global_Interrupt();

    while (1) {
        uint8 cmd = UART_recieveByte();
        switch (cmd) {
            case 'C': // CHECK_PW
                UART_sendByte(isPasswordSet ? 'S' : 'N');
                break;
            case 'S': // SET_PW
                handleSetPassword();
                break;
            case 'V': // VERIFY_PW
                handleVerifyPassword();
                break;
            case 'O': // OPEN_DOOR
                handleOpenDoor();
                break;
            case 'L': // LOCK_SYSTEM
                handleLockSystem();
                break;
        }
    }
}
