/******************************************************************************
 *
 * Module: Control_ECU main Program
 *
 * File Name: main.c
 *
 * Description: Application of Control_ECU
 *
 * Author: Zeyad Ehab
 *
 *******************************************************************************/

// Include necessary header files for hardware interfacing and delays
#include "external_eeprom.h"  // Library for external EEPROM operations (password storage)
#include "pir.h"              // Library for PIR sensor operations (motion detection)
#include "dc_motor.h"         // Library for DC motor control (door operation)
#include "buzzer.h"           // Library for buzzer control (alarm during lock)
#include "uart.h"             // Library for UART communication with HMI_ECU
#include "timer.h"            // Library for timer operations (motor and buzzer timing)
#include "interrupt.h"        // Library for interrupt handling (timer interrupts)
#include "twi.h"              // Library for TWI (I2C) communication with EEPROM
#include <avr/io.h>           // AVR I/O definitions for direct register access
#include <util/delay.h>       // Delay functions for timing (e.g., EEPROM operations)

// Define constants for password storage and EEPROM addresses
#define PASSWORD_LENGTH 5        // Length of the password (5 digits)
#define PASSWORD_ADDRESS 0x00    // Starting address in EEPROM to store the password (0x00 to 0x04)
#define PW_FLAG_ADDRESS 0x05     // EEPROM address to store the password set flag
#define PW_SET_FLAG 0x55         // Flag value indicating a password is set

// Declare global variables
uint8 storedPassword[PASSWORD_LENGTH];  // Array to store the password loaded from EEPROM
uint8 isPasswordSet = 0;                // Flag to indicate if a password is set (1 = set, 0 = not set)
uint8 g_ticks = 0;                      // Counter for timer ticks (used for motor and buzzer timing)

// UART configuration: 8 data bits, no parity, 1 stop bit, 9600 baud rate
UART_ConfigType uartConfig = {EIGHT_BITS, DISABLE, ONE_BIT, 9600};

// Timer configuration: Timer1, compare mode, ~3-second ticks
// F_CPU assumed 8 MHz, prescaler 1024, compare value 23436
// Tick frequency: 8000000 / (1024 * (23436 + 1)) ≈ 0.333 Hz, period ≈ 3 seconds
Timer_ConfigType timerConfig = {0, 23436, TIMER1, F_CPU_CLOCK_1024, COMPARE_MODE};

// TWI (I2C) configuration for EEPROM communication
TWI_ConfigType twiConfig = {I2C_Master_Address, I2C_Bit_Rate};

// Function to flush the UART receive buffer
void flushUARTBuffer() {
    // Loop while RXC flag is set (data available in UART buffer)
    while (UCSRA & (1 << RXC)) {
        (void)UDR;  // Read and discard data from UDR register to clear buffer
    }
}

// Function to load the password from external EEPROM
void loadPassword() {
    uint8 tempData;  // Temporary variable to store read data
    // Read the password set flag from EEPROM
    if (EEPROM_readByte(PW_FLAG_ADDRESS, &tempData) != SUCCESS || tempData != PW_SET_FLAG) {
        isPasswordSet = 0;  // If flag read fails or flag isn't set, mark password as not set
        return;             // Exit the function early
    }
    // Read each byte of the password from EEPROM
    for (uint8 i = 0; i < PASSWORD_LENGTH; i++) {
        // Read byte from EEPROM at PASSWORD_ADDRESS + i
        if (EEPROM_readByte(PASSWORD_ADDRESS + i, &tempData) != SUCCESS) {
            isPasswordSet = 0;  // If read fails, mark password as not set
            return;             // Exit the function early
        }
        storedPassword[i] = tempData;  // Store the read byte in the password array
        _delay_ms(10);                 // Delay to ensure EEPROM read timing
    }
    _delay_ms(10);                     // Additional delay after reading all bytes
    isPasswordSet = 1;                 // Mark password as set if all reads succeed
}

// Function to save a password to external EEPROM
void savePassword(const uint8* pw) {
    uint8 success = 1;  // Flag to track success of EEPROM writes
    // Write each byte of the password to EEPROM
    for (uint8 i = 0; i < PASSWORD_LENGTH; i++) {
        // Write byte to EEPROM at PASSWORD_ADDRESS + i
        if (EEPROM_writeByte(PASSWORD_ADDRESS + i, pw[i]) != SUCCESS) {
            success = 0;  // Mark failure if write fails
            break;        // Exit the loop early
        }
        _delay_ms(10);    // Delay to ensure EEPROM write cycle completes
    }
    // Write the password set flag to EEPROM
    if (EEPROM_writeByte(PW_FLAG_ADDRESS, PW_SET_FLAG) != SUCCESS) {
        success = 0;      // Mark failure if flag write fails
    }
    _delay_ms(10);        // Delay after writing the flag
    // Check the result of the write operations
    if (success) {
        isPasswordSet = 1;  // Mark password as set if all writes succeed
    } else {
        isPasswordSet = 0;  // Mark password as not set if any write fails
        UART_sendByte('E'); // Send 'E' to HMI_ECU to indicate error
    }
}

// Function to compare two passwords
uint8 comparePasswords(uint8* pw1, uint8* pw2) {
    // Compare each byte of the two passwords
    for (uint8 i = 0; i < PASSWORD_LENGTH; i++) {
        if (pw1[i] != pw2[i]) return 0;  // Return 0 if any bytes don't match
    }
    return 1;  // Return 1 if all bytes match
}

// Function to handle password setting request from HMI_ECU
void handleSetPassword() {
    uint8 pw1[PASSWORD_LENGTH], pw2[PASSWORD_LENGTH];  // Arrays for initial and confirmation passwords
    // Receive the initial password from HMI_ECU
    for (uint8 i = 0; i < PASSWORD_LENGTH; i++) {
        pw1[i] = UART_recieveByte();  // Read each byte of the initial password
    }
    // Receive the confirmation password from HMI_ECU
    for (uint8 i = 0; i < PASSWORD_LENGTH; i++) {
        pw2[i] = UART_recieveByte();  // Read each byte of the confirmation password
    }
    // Compare the two passwords
    if (comparePasswords(pw1, pw2)) {
        savePassword(pw1);  // Save the password to EEPROM if they match
        // Check if the save operation was successful
        if (isPasswordSet) {
            UART_sendByte('S');  // Send 'S' to HMI_ECU to indicate success
        } else {
            UART_sendByte('F');  // Send 'F' to HMI_ECU to indicate failure
        }
    } else {
        UART_sendByte('N');  // Send 'N' to HMI_ECU to indicate passwords didn't match
    }
}

// Function to handle password verification request from HMI_ECU
void handleVerifyPassword() {
    flushUARTBuffer();  // Clear UART buffer to ensure clean data
    uint8 pw[PASSWORD_LENGTH];  // Array to store the received password
    // Receive the password from HMI_ECU
    for (uint8 i = 0; i < PASSWORD_LENGTH; i++) {
        pw[i] = UART_recieveByte();  // Read each byte of the password
    }
    loadPassword();  // Load the stored password from EEPROM
    // Compare the received password with the stored one
    if (isPasswordSet && comparePasswords(pw, storedPassword)) {
        UART_sendByte('M');  // Send 'M' to HMI_ECU to indicate a match
    } else {
        UART_sendByte('N');  // Send 'N' to HMI_ECU to indicate no match
    }
}

// Variables for motor and buzzer timing
volatile uint8 motorDone = 1;  // Flag to indicate motor operation is complete
volatile uint8 LockDone = 0;   // Flag to indicate lock (buzzer) duration is complete

// Timer callback function for motor and buzzer timing
void motorCallback() {
    g_ticks++;  // Increment tick counter
    // Check if motor operation should complete
    if (g_ticks == 1 && motorDone == 0) {  // 1 tick * 3s/tick = 3s for motor
        motorDone = 1;  // Set flag to indicate motor operation is complete
        g_ticks = 0;    // Reset tick counter
    }
    // Check if buzzer operation should complete
    if (g_ticks == 2 && LockDone == 0) {  // 2 ticks * 3s/tick = 6s for buzzer
        LockDone = 1;  // Set flag to indicate buzzer duration is complete
        g_ticks = 0;   // Reset tick counter
    }
}

// Function to handle door opening process
void handleOpenDoor() {
    DcMotor_Rotate(CLOCKWISE);  // Start motor to rotate clockwise (unlock door)
    UART_sendByte('U');         // Send 'U' to HMI_ECU to indicate unlocking
    motorDone = 0;              // Reset motor flag to start timing
    g_ticks = 0;                // Reset tick counter
    Timer_setCallback(motorCallback, TIMER1);  // Set timer callback function
    Timer_init(&timerConfig);   // Start Timer1 with configured settings
    while (!motorDone);         // Wait until motor operation completes (3 seconds)
    DcMotor_Rotate(STOP);       // Stop the motor

    UART_sendByte('W');         // Send 'W' to HMI_ECU to indicate waiting for people
    while (PIR_getState() == 1);  // Wait until no motion is detected by PIR sensor

    DcMotor_Rotate(ANTI_CLOCKWISE);  // Start motor to rotate counterclockwise (lock door)
    UART_sendByte('L');              // Send 'L' to HMI_ECU to indicate locking
    motorDone = 0;                   // Reset motor flag to start timing
    g_ticks = 0;                     // Reset tick counter
    Timer_setCallback(motorCallback, TIMER1);  // Set timer callback function
    Timer_init(&timerConfig);        // Start Timer1 with configured settings
    while (!motorDone);              // Wait until motor operation completes (3 seconds)
    DcMotor_Rotate(STOP);            // Stop the motor
    Timer_delnit(TIMER1);            // Deinitialize the timer (typo: should be Timer_deinit)
    UART_sendByte('D');              // Send 'D' to HMI_ECU to indicate door operation is done
}

// Function to handle system lock (buzzer activation)
void handleLockSystem() {
    Buzzer_on();                // Turn on the buzzer to indicate lock
    LockDone = 0;               // Reset lock flag to start timing
    g_ticks = 0;                // Reset tick counter
    Timer_setCallback(motorCallback, TIMER1);  // Set timer callback function
    Timer_init(&timerConfig);   // Start Timer1 with configured settings
    while(!LockDone);           // Wait until lock duration completes (6 seconds)
    Timer_delnit(TIMER1);       // Deinitialize the timer (typo: should be Timer_deinit)
    Buzzer_off();               // Turn off the buzzer
}

// Timer1 Compare Match A interrupt service routine
ISR(TIMER1_COMPA_vect) {
    // Empty since the timer callback (motorCallback) handles the logic
}

int main(void) {
    // Initialize hardware peripherals
    TWI_init(&twiConfig);       // Initialize TWI (I2C) for EEPROM communication
    PIR_init();                 // Initialize PIR sensor
    DcMotor_Init();             // Initialize DC motor
    Buzzer_init();              // Initialize buzzer
    UART_init(&uartConfig);     // Initialize UART with configured settings
    Timer_init(&timerConfig);   // Initialize Timer1 with configured settings
    loadPassword();             // Load the password from EEPROM at startup
    Enable_Global_Interrupt();  // Enable global interrupts for timer

    // Main loop to handle commands from HMI_ECU
    while (1) {
        uint8 cmd = UART_recieveByte();  // Receive command from HMI_ECU
        switch (cmd) {
            case 'C':  // Command to check if password is set
                UART_sendByte(isPasswordSet ? 'S' : 'N');  // Send 'S' if set, 'N' if not
                break;

            case 'S':  // Command to set a new password
                handleSetPassword();  // Call function to handle password setting
                break;

            case 'V':  // Command to verify a password
                handleVerifyPassword();  // Call function to handle password verification
                break;

            case 'O':  // Command to open the door
                handleOpenDoor();  // Call function to handle door opening
                break;

            case 'L':  // Command to lock the system
                handleLockSystem();  // Call function to handle system lock
                break;
        }
    }
}
