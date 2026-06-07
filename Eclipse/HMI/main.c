/******************************************************************************
 *
 * Module: HMI_ECU
 *
 * File Name: main.c
 *
 * Description: HMI_ECU Application
 *
 * Author: Zeyad Ehab
 *
 *******************************************************************************/

// Include necessary header files for hardware interfacing and delays
#include "lcd.h"         // Library for LCD operations (display messages)
#include "keypad.h"      // Library for keypad input (user interaction)
#include "uart.h"        // Library for UART communication with Control_ECU
#include "timer.h"       // Library for timer operations (used in lock system)
#include "interrupt.h"   // Library for interrupt handling (timer interrupts)
#include <avr/io.h>      // AVR I/O definitions for direct register access
#include <util/delay.h>  // Delay functions for timing (e.g., debounce)

// Define constants for password length and keypad keys
#define PASSWORD_LENGTH 5        // Length of the password (5 digits)
#define ENTER_KEY '='            // Key to confirm password entry
#define OPEN_DOOR_KEY '*'        // Key to select "Open Door" option
#define CHANGE_PASSWORD_KEY '%'  // Key to select "Change Password" option

// Enum to define the possible states of the HMI_ECU state machine
typedef enum {
    INIT,               // Initial state to check if password is set
    SET_PASSWORD,       // State to set a new password
    MAIN_OPTIONS,       // State to display main options (open door or change password)
    ENTER_PASSWORD,     // State to enter password for verification
    OPEN_DOOR,          // State to handle door opening process
    CHANGE_PASSWORD,    // State to handle password change process
    LOCK_SYSTEM         // State to lock the system after failed attempts
} State;

// Declare global variables
State currentState = INIT;                      // Current state of the HMI_ECU, starts at INIT
uint8 password[PASSWORD_LENGTH];                // Array to store the initial password entered by user
uint8 confirmPassword[PASSWORD_LENGTH];         // Array to store the confirmation password for verification
uint8 enteredPassword[PASSWORD_LENGTH];         // Array to store the password entered for verification
uint8 savedPassword[PASSWORD_LENGTH];           // Array to store the confirmed password for display
uint8 selectedOption;                           // Stores the user's selected option ('O' for open door, 'C' for change password)
uint8 attempts = 0;                             // Counter for failed password verification attempts

// UART configuration: 8 data bits, no parity, 1 stop bit, 9600 baud rate
UART_ConfigType uartConfig = {EIGHT_BITS, DISABLE, ONE_BIT, 9600};

// Timer configuration for lock system: Timer1, compare mode, ~3-second ticks
// F_CPU assumed 8 MHz, prescaler 1024, compare value 23436
// Tick frequency: 8000000 / (1024 * (23436 + 1)) ≈ 0.333 Hz, period ≈ 3 seconds
Timer_ConfigType timerConfig = {0, 23436, TIMER1, F_CPU_CLOCK_1024, COMPARE_MODE};

// Function to read a password from the keypad
void readPassword(uint8* pw) {
    uint8 i = 0;                        // Index to track number of digits entered
    LCD_moveCursor(1, 0);               // Move LCD cursor to row 1, column 0 to display password input

    // Loop until all 5 digits of the password are entered
    while(i < PASSWORD_LENGTH) {
        uint8 key = KEYPAD_getPressedKey();  // Get the pressed key from the keypad
        if (key >= 0 && key <= 9) {          // Check if the key is a number (0-9)
            pw[i] = key;                     // Store the digit in the password array
            LCD_displayCharacter('*');       // Display '*' on LCD to mask the digit
            i++;                             // Increment the digit counter
        }
        _delay_ms(300);                      // Delay for keypad debounce to avoid multiple reads of the same press
    }
    // Wait for the user to press the ENTER_KEY ('=') to confirm the password entry
    while (KEYPAD_getPressedKey() != ENTER_KEY);
}

// Function to send a command to the Control_ECU via UART
void sendCommand(const uint8 cmd) {
    UART_sendByte(cmd);                 // Send a single byte command (e.g., 'S', 'V', 'O', 'L', 'C')
}

// Function to send a password to the Control_ECU via UART
void sendPassword(const uint8* pw) {
    for (uint8 i = 0; i < PASSWORD_LENGTH; i++) {
        UART_sendByte(pw[i]);           // Send each digit of the password as a byte
    }
}

// Function to receive a response from the Control_ECU via UART
uint8 receiveResponse() {
    return UART_recieveByte();          // Receive a single byte response (e.g., 'S', 'N', 'M', 'U', 'W', 'L', 'D')
}

// Function to display a message on the LCD, clearing the previous content
void displayMessage(const char* msg) {
    LCD_clearScreen();                  // Clear the LCD screen
    LCD_displayString(msg);             // Display the provided message
}

// Function to handle the password setting process
void handleSetPassword() {
    displayMessage("Enter Password:");  // Prompt user to enter a new password
    readPassword(password);             // Read the initial password into the password array
    displayMessage("Confirm Password:"); // Prompt user to confirm the password
    readPassword(confirmPassword);      // Read the confirmation password
    LCD_clearScreen();                  // Clear the LCD after password entry

    // Send the 'S' command to Control_ECU to initiate password setting
    sendCommand((uint8)'S');
    sendPassword(password);             // Send the initial password
    sendPassword(confirmPassword);      // Send the confirmation password
    uint8 response = receiveResponse(); // Receive response from Control_ECU

    // Check the response from Control_ECU
    if (response == 'S') {              // 'S' means passwords matched and were saved successfully
        currentState = MAIN_OPTIONS;    // Move to the main options state
    } else {                            // 'N' means passwords didn't match
        displayMessage("Mismatch! Try again"); // Display error message
        _delay_ms(1000);                // Delay to allow user to read the message
        // State remains SET_PASSWORD, so user can try again
    }
}

// Function to display and handle the main options menu
void handleMainOptions() {
    // Display options on LCD
    LCD_displayStringRowColumn(0, 0, "* : Open Door   ");  // Option to open door on row 0
    LCD_displayStringRowColumn(1, 0, "% : Change PW   ");  // Option to change password on row 1

    uint8 key;                          // Variable to store the pressed key
    // Wait for user to select an option
    do {
        key = KEYPAD_getPressedKey();   // Get the pressed key
        _delay_ms(300);                 // Debounce delay
    } while (key != OPEN_DOOR_KEY && key != CHANGE_PASSWORD_KEY); // Repeat until '*' or '%' is pressed

    // Set the selected option based on the key pressed
    selectedOption = (key == OPEN_DOOR_KEY) ? 'O' : 'C'; // 'O' for open door, 'C' for change password
    currentState = ENTER_PASSWORD;      // Move to password entry state for verification
}

// Function to handle password entry for verification before proceeding
void handleEnterPassword() {
    displayMessage("Enter Password:");  // Prompt user to enter the password
    readPassword(enteredPassword);      // Read the entered password
    sendCommand((uint8)'V');            // Send 'V' command to Control_ECU to verify password
    _delay_ms(10);                      // Small delay to ensure Control_ECU is ready
    sendPassword(enteredPassword);      // Send the entered password
    uint8 response = receiveResponse(); // Receive response from Control_ECU

    // Check the verification response
    if (response == 'M') {              // 'M' means password matched
        attempts = 0;                   // Reset failed attempts counter
        		// Proceed based on selected option
        currentState = (selectedOption == 'O') ? OPEN_DOOR : CHANGE_PASSWORD;
    } else {                            // 'N' means password didn't match
        attempts++;                     // Increment failed attempts counter
        if (attempts < 3) {             // Allow up to 3 attempts
            displayMessage("Wrong PW! Try again"); // Display error message
            _delay_ms(1000);            // Delay to allow user to read the message
            // State remains ENTER_PASSWORD, so user can try again
        } else {                        // After 3 failed attempts
            sendCommand((uint8)'L');    // Send 'L' command to lock the system
            currentState = LOCK_SYSTEM; // Move to lock system state
        }
    }
}

// Function to handle the door opening process
void handleOpenDoor() {
    sendCommand((uint8)'O');            			// Send 'O' command to Control_ECU to open the door
    _delay_ms(10);                      			// Small delay to ensure Control_ECU is ready
    while (1) {                         			// Loop to handle door operation stages
        uint8 status = receiveResponse(); 			// Receive status updates from Control_ECU
        if (status == 'U') displayMessage("Door is Unlocking"); // Display unlocking message
        else if (status == 'W') displayMessage("Wait for people "); // Display waiting message
        else if (status == 'L') displayMessage("Door is Locking  "); // Display locking message
        else if (status == 'D') {       // 'D' means door operation is done
            currentState = MAIN_OPTIONS; // Return to main options state
            break;                      // Exit the loop
        }
    }
}

// Variables for lock system timing
uint8 g_ticks = 0;                      // Counter for timer ticks
volatile uint8 lockDone = 0;            // Flag to indicate lock duration is complete

// Timer callback function for lock system
void lockCallback() {
    g_ticks++;                          // Increment tick counter
    if (g_ticks == 2) {                 // 2 ticks * 3s/tick = 6s (intended for 12s in Control_ECU)
        lockDone = 1;                   // Set flag to indicate lock duration is complete
        g_ticks = 0;                    // Reset tick counter
    }
}

// Function to handle system lock after failed attempts
void handleLockSystem() {
    displayMessage("Error: Locked");    // Display lock message
    lockDone = 0;                       // Reset lock flag
    Timer_setCallback(lockCallback, TIMER1); // Set the timer callback function
    Timer_init(&timerConfig);           // Start Timer1 with configured settings
    while (!lockDone);                  // Wait until lock duration is complete
    currentState = MAIN_OPTIONS;        // Return to main options state
}

// Timer1 Compare Match A interrupt service routine
ISR(TIMER1_COMPA_vect) {
    // Empty since the timer callback (lockCallback) handles the logic
}

int main(void) {
    // Initialize hardware peripherals
    LCD_init();                         // Initialize the LCD
    UART_init(&uartConfig);             // Initialize UART with configured settings
    Enable_Global_Interrupt();          // Enable global interrupts for timer

    // Main state machine loop
    while (1) {
        switch (currentState) {
            case INIT:                  // Initial state to check if password is set
                sendCommand((uint8)'C'); // Send 'C' command to check password status
                if (receiveResponse() == 'N') { // 'N' means no password is set
                    currentState = SET_PASSWORD; // Move to set password state
                } else {                // 'S' means password is already set
                    currentState = MAIN_OPTIONS; // Move to main options state
                }
                break;

            case SET_PASSWORD:          // State to set a new password
                handleSetPassword();    // Call function to handle password setting
                break;

            case MAIN_OPTIONS:          // State to display and select main options
                handleMainOptions();    // Call function to handle main options
                break;

            case ENTER_PASSWORD:        // State to enter password for verification
                handleEnterPassword();  // Call function to handle password entry
                break;

            case OPEN_DOOR:             // State to handle door opening
                handleOpenDoor();       // Call function to handle door operation
                break;

            case CHANGE_PASSWORD:       // State to handle password change
                handleSetPassword();    // Reuse set password function for changing password
                break;

            case LOCK_SYSTEM:           // State to handle system lock
                handleLockSystem();     // Call function to handle lock system
                break;
        }
    }

    return 0;                           // Main function return (never reached in embedded systems)
}
