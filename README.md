# 🔐 AVR Door Locker Security System

A dual-microcontroller embedded security system built on **ATmega32** using bare-metal C. The system implements a password-protected electronic door lock with real-time motor control, motion detection, and a 1-minute lockout mechanism — designed as a final diploma project in embedded systems.

---

## 📌 Project Overview

The system is split across two ATmega32 MCUs communicating over UART:

| MCU | Role | Key Peripherals |
|---|---|---|
| **HMI ECU** | User interface & input handling | LCD, 4×4 Keypad, UART, Timer1 |
| **Control ECU** | Logic, security & actuation | DC Motor, Buzzer, PIR Sensor, EEPROM, UART, TWI/I2C, Timer1 |

The user sets a 5-digit password which is stored persistently in external EEPROM. On each door-open attempt, the entered password is verified by the Control ECU. After 3 consecutive wrong attempts, the system locks for 60 seconds and sounds the buzzer.

---

## 🏗️ System Architecture

```
┌─────────────────────────────────┐     UART (9600 baud)     ┌─────────────────────────────────┐
│           HMI ECU               │ ◄───────────────────────► │         Control ECU             │
│         (ATmega32)              │                            │         (ATmega32)              │
│                                 │                            │                                 │
│  ┌──────────┐  ┌─────────────┐  │                            │  ┌──────────┐  ┌────────────┐  │
│  │   LCD    │  │ 4×4 Keypad  │  │                            │  │ DC Motor │  │  Buzzer    │  │
│  └──────────┘  └─────────────┘  │                            │  └──────────┘  └────────────┘  │
│  ┌──────────┐  ┌─────────────┐  │                            │  ┌──────────┐  ┌────────────┐  │
│  │  Timer1  │  │  Interrupt  │  │                            │  │   PIR    │  │  EEPROM    │  │
│  └──────────┘  └─────────────┘  │                            │  │ Sensor   │  │ (I2C/TWI) │  │
└─────────────────────────────────┘                            │  └──────────┘  └────────────┘  │
                                                               └─────────────────────────────────┘
```

---

## 🔄 System State Machine (HMI ECU)

```
         ┌─────────┐
         │  INIT   │ ── Check if password exists in EEPROM
         └────┬────┘
      ┌───────┴────────┐
      ▼                ▼
┌──────────┐    ┌──────────────┐
│   SET_   │    │    MAIN_     │
│ PASSWORD │    │   OPTIONS    │◄──────────────────┐
└────┬─────┘    └──────┬───────┘                   │
     │                 │  * = Open Door            │
     │                 │  % = Change Password      │
     │          ┌──────▼──────┐                    │
     │          │   ENTER_    │                    │
     │          │  PASSWORD   │                    │
     │          └──────┬──────┘                    │
     │         Match   │   3 fails                 │
     │      ┌──────────┴────────┐                  │
     │      ▼                   ▼                  │
     │  ┌───────┐         ┌──────────┐             │
     └─►│ OPEN_ │         │  LOCK_   │─────────────┘
        │  DOOR │         │  SYSTEM  │  (60s timeout)
        └───────┘         └──────────┘
```

---

## 📁 Repository Structure

```
AVR-Door-Locker-Security-System/
│
├── HMI/                        # Human Machine Interface ECU
│   ├── main.c                  # State machine & UI flow
│   ├── lcd.c / lcd.h           # LCD 16x2 driver
│   ├── keypad.c / keypad.h     # 4x4 matrix keypad driver
│   ├── uart.c / uart.h         # UART driver (configurable)
│   ├── timer.c / timer.h       # Timer1 driver (CTC mode)
│   ├── gpio.c / gpio.h         # GPIO abstraction layer
│   ├── interrupt.h             # Global interrupt macros
│   ├── std_types.h             # Standard type definitions
│   └── common_macros.h         # Common utility macros
│
├── Control_ECU/                # Control & Security ECU
│   ├── main.c                  # Command handler & security logic
│   ├── external_eeprom.c/.h    # AT24C16 EEPROM driver (via I2C)
│   ├── twi.c / twi.h          # TWI/I2C driver (400 kHz)
│   ├── dc_motor.c / dc_motor.h # DC motor driver (H-Bridge + PWM)
│   ├── buzzer.c / buzzer.h     # Buzzer driver
│   ├── PIR.c / PIR.h          # PIR motion sensor driver
│   ├── uart.c / uart.h         # UART driver
│   ├── timer.c / timer.h       # Timer1 driver
│   ├── PWM.c / PWM.h          # PWM driver (motor speed control)
│   ├── gpio.c / gpio.h         # GPIO abstraction layer
│   ├── interrupt.h             # Global interrupt macros
│   ├── std_types.h             # Standard type definitions
│   └── common_macros.h         # Common utility macros
│
├── .gitignore
└── README.md
```

---

## ⚙️ Hardware Components

| Component | Purpose | Interface |
|---|---|---|
| 2x ATmega32 | Main MCUs (HMI + Control) | — |
| 16x2 LCD | Display messages & prompts | GPIO (4-bit mode) |
| 4x4 Matrix Keypad | User password input | GPIO |
| DC Motor + H-Bridge (L293D) | Door lock/unlock mechanism | PWM + GPIO |
| PIR Motion Sensor | Detect if person has passed | GPIO |
| AT24C16 EEPROM | Persistent password storage | I2C / TWI |
| Buzzer | Security alert (lockout) | GPIO |

---

## 🔌 Pin Configuration

### HMI ECU (ATmega32)

| Peripheral | Port / Pin |
|---|---|
| LCD (Data + Control) | PORTA / PORTB |
| Keypad (Rows + Cols) | PORTB |
| UART TX | PORTD – PIN1 |
| UART RX | PORTD – PIN0 |

### Control ECU (ATmega32)

| Peripheral | Port / Pin |
|---|---|
| DC Motor IN1 | PORTD – PIN6 |
| DC Motor IN2 | PORTD – PIN7 |
| Motor Enable (PWM) | PORTB – PIN3 |
| Buzzer | PORTC – PIN7 |
| PIR Sensor | PORTC – PIN2 |
| I2C SCL (EEPROM) | PORTC – PIN0 |
| I2C SDA (EEPROM) | PORTC – PIN1 |
| UART TX | PORTD – PIN1 |
| UART RX | PORTD – PIN0 |

---

## 🛠️ Tools & Environment

| Tool | Detail |
|---|---|
| IDE | Eclipse CDT (AVR Eclipse Plugin) |
| Compiler | AVR-GCC |
| Target MCU | ATmega32 @ 8 MHz |
| Programmer | AVR ISP / USBasp |
| Language | C (bare-metal, no RTOS) |
| Communication | UART @ 9600 baud, I2C @ 400 kHz |

---

## 🚀 Getting Started

### Prerequisites

- **Eclipse CDT** with the [AVR Eclipse Plugin](https://avr-eclipse.sourceforge.net/wiki/index.php/The_AVR_Eclipse_Plugin)
- **AVR-GCC toolchain** installed and configured
- **AVRDude** for flashing

### Import Projects into Eclipse

1. Open Eclipse CDT.
2. Go to **File → Import → General → Existing Projects into Workspace**.
3. Browse to the cloned repository root.
4. Select both **HMI** and **Control_ECU** projects → click **Finish**.

### Build & Flash

1. Right-click each project → **Build Project** to compile.
2. Flash each `.elf` to its respective ATmega32:
   ```bash
   avrdude -c usbasp -p m32 -U flash:w:HMI/Debug/HMI.elf
   avrdude -c usbasp -p m32 -U flash:w:Control_ECU/Debug/Control_ECU.elf
   ```
3. Connect the two MCUs via UART (TX→RX cross-wired, shared GND).

---

## 📋 UART Command Protocol

The two MCUs communicate via a simple single-byte command protocol:

| Command (HMI → Control) | Meaning |
|---|---|
| `'C'` | Check if a password is already stored |
| `'S'` | Set new password (followed by 10 bytes: password + confirm) |
| `'V'` | Verify entered password (followed by 5 bytes) |
| `'O'` | Open door sequence |
| `'L'` | Trigger lockout (buzzer for 60 seconds) |

| Response (Control → HMI) | Meaning |
|---|---|
| `'S'` | Password saved successfully |
| `'N'` | Password not set / mismatch |
| `'M'` | Password matched |
| `'U'` | Door unlocking (motor CW, 15s) |
| `'W'` | Waiting for person to pass (PIR check) |
| `'L'` | Door locking (motor CCW, 15s) |
| `'D'` | Door sequence complete |

---

## 🔒 Security Features

- **5-digit PIN** stored persistently in external EEPROM (survives power loss)
- **Password confirmation** required on first setup and on every change
- **3-attempt lockout** — system locks for 60 seconds after 3 consecutive wrong entries
- **Audible alert** — buzzer sounds throughout the entire lockout period
- **Motion-aware locking** — door only re-locks after the PIR sensor confirms no motion

---

## 📄 License

This project is licensed under the [MIT License](LICENSE).

---

## 👤 Author

**Ziyad Ehab**  
Embedded Systems Diploma  
[GitHub](https://github.com/Ziyad-ehab)
