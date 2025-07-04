# IoT Prototype System - STM32G071RB with BME680

## Project Overview
This is an IoT prototype system based on the STM32G071RB Nucleo board that collects environmental data from a BME680 sensor and provides a command interface via UART.

## Hardware Setup
- **MCU**: STM32G071RB Nucleo board
- **Sensor**: BME680 (Temperature, Pressure, Humidity, Gas)

### Pin Connections

#### I2C1 (BME680 Sensor)
| Nucleo Pin | Function | BME680 Pin | Notes |
|------------|----------|------------|-------|
| PB8        | I2C1_SCL | SCL        | Clock line |
| PB9        | I2C1_SDA | SDA        | Data line |
| 3.3V       | VCC      | VCC        | Power supply |
| GND        | GND      | GND        | Ground |

#### UART Connections
| Nucleo Pin | Function | USB-UART Adapter | Notes |
|------------|----------|------------------|-------|
| PA2        | USART2_TX | RX (Terminal 1) | Primary UART |
| PA3        | USART2_RX | TX (Terminal 1) | Primary UART |
| PA0        | USART4_TX | RX (Terminal 2) | Secondary UART |
| PA1        | USART4_RX | TX (Terminal 2) | Secondary UART |

#### Status LED
| Nucleo Pin | Function | Notes |
|------------|----------|-------|
| PA5        | LED      | Blinks when system is running |

### Wiring Diagram
```
NUCLEO-G071RB          BME680 Sensor
┌─────────────┐        ┌─────────────┐
│             │        │             │
│ PB8 ────────┼────────┤ SCL         │
│             │        │             │
│ PB9 ────────┼────────┤ SDA         │
│             │        │             │
│ 3.3V ───────┼────────┤ VCC         │
│             │        │             │
│ GND ────────┼────────┤ GND         │
│             │        │             │
└─────────────┘        └─────────────┘

NUCLEO-G071RB          USB-UART Adapter 1
┌─────────────┐        ┌─────────────┐
│             │        │             │
│ PA2 ────────┼────────┤ RX          │
│             │        │             │
│ PA3 ────────┼────────┤ TX          │
│             │        │             │
└─────────────┘        └─────────────┘

NUCLEO-G071RB          USB-UART Adapter 2
┌─────────────┐        ┌─────────────┐
│             │        │             │
│ PA0 ────────┼────────┤ RX          │
│             │        │             │
│ PA1 ────────┼────────┤ TX          │
│             │        │             │
└─────────────┘        └─────────────┘
```

### Important Notes
- **Pull-up Resistors**: 4.7kΩ pull-up resistors are recommended on SCL and SDA lines
- **I2C Address**: BME680 default address is 0x76
- **Power Supply**: BME680 requires 3.3V power supply
- **Baud Rate**: Both UARTs configured at 115200 baud

## Features Implemented

### 1. BME680 Sensor Interface
- I2C communication with BME680 sensor
- Temperature, pressure, and humidity reading
- Sensor initialization and configuration
- Error handling and status reporting

### 2. Command Interface System
- **Dual UART Support**: Command processing via both USART2 and USART4
- Interactive command prompt on both interfaces
- Real-time command parsing and execution
- Help system with available commands
- Independent command sessions for each UART

### 3. Available Commands
- `start` - Initialize the command system
- `read temperature` - Read temperature from BME680
- `read pressure` - Read pressure from BME680
- `read humidity` - Read humidity from BME680
- `test sensor` - Test BME680 sensor functionality
- `sum <num1> <num2>` - Add two numbers
- `sub <num1> <num2>` - Subtract num2 from num1
- `mul <num1> <num2>` - Multiply two numbers
- `div <num1> <num2>` - Divide num1 by num2
- `help` - Show available commands

## Software Architecture

### Files Structure
```
Core/
├── Inc/
│   ├── bme680_interface.h    # BME680 sensor interface
│   ├── command_interface.h   # Command processing system
│   ├── bme68x.h             # Bosch BME680 library
│   ├── bme68x_defs.h        # BME680 definitions
│   └── main.h               # Main application header
├── Src/
│   ├── bme680_interface.c   # BME680 implementation
│   ├── command_interface.c  # Command system implementation
│   ├── bme68x.c            # Bosch BME680 library
│   └── main.c              # Main application
```

### Key Components

#### BME680 Interface (`bme680_interface.c`)
- `bme680_i2c_read()` - I2C read function for BME680
- `bme680_i2c_write()` - I2C write function for BME680
- `bme680_init_sensor()` - Initialize and configure BME680
- `bme680_read_sensor_data()` - Read sensor data
- `bme680_test_sensor()` - Test sensor functionality

#### Command Interface (`command_interface.c`)
- `command_interface_init()` - Initialize command system
- `command_interface_process()` - Process incoming commands
- `command_interface_handle_command()` - Parse and execute commands
- Individual command handlers for each function

## Usage Instructions

### 1. Hardware Connection
1. Connect BME680 sensor to I2C1:
   - SCL → PB8
   - SDA → PB9
   - VCC → 3.3V
   - GND → GND

2. Connect first USB-UART adapter to USART2:
   - TX → PA2
   - RX → PA3

3. Connect second USB-UART adapter to USART4:
   - TX → PA0
   - RX → PA1

### 2. Software Setup
1. Compile and flash the project to STM32G071RB
2. Open two terminal programs (PuTTY, Tera Term, etc.) - one for each UART
3. Configure both terminals:
   - Baud rate: 115200
   - Data bits: 8
   - Stop bits: 1
   - Parity: None
   - Flow control: None
4. Connect to the appropriate COM ports for each USB-UART adapter

### 3. System Operation
1. Power on the system
2. Wait for initialization messages on both UARTs
3. Type `start` on either UART to begin command interface
4. Use `help` to see available commands
5. Execute sensor readings and mathematical operations
6. Both UARTs operate independently - you can use either or both simultaneously

## Example Session

### USART2 Session:
```
========================================
IoT Prototype System - STM32G071RB
========================================
System Clock: 16 MHz
I2C1 Configuration: PB8 (SCL), PB9 (SDA)
USART2: PA2 (TX), PA3 (RX) - 115200 baud
USART4: PA0 (TX), PA1 (RX) - 115200 baud
LED Status: PA5
========================================

Checking BME680 sensor presence...
✓ BME680 sensor detected on I2C bus (Address: 0x76)
Initializing BME680 sensor...
✓ BME680 sensor initialized successfully
  - Temperature oversampling: 1x
  - Pressure oversampling: 1x
  - Humidity oversampling: 1x
  - Gas sensor: Disabled

IoT Prototype System Ready (USART2)
Type 'start' to begin
> start
System started! Type 'help' for available commands.
> help
=== Available Commands ===
read temperature - Read temperature from BME680
read pressure   - Read pressure from BME680
read humidity   - Read humidity from BME680
test sensor     - Test BME680 sensor
sum <num1> <num2> - Add two numbers
sub <num1> <num2> - Subtract num2 from num1
mul <num1> <num2> - Multiply two numbers
div <num1> <num2> - Divide num1 by num2
help            - Show this help menu
========================
> read temperature
Temperature: 23.45°C
> sum 5 3
5.00 + 3.00 = 8.00
```

### USART4 Session (simultaneous):
```
========================================
IoT Prototype System - STM32G071RB
========================================
System Clock: 16 MHz
I2C1 Configuration: PB8 (SCL), PB9 (SDA)
USART2: PA2 (TX), PA3 (RX) - 115200 baud
USART4: PA0 (TX), PA1 (RX) - 115200 baud
LED Status: PA5
========================================

Checking BME680 sensor presence...
✓ BME680 sensor detected on I2C bus (Address: 0x76)
Initializing BME680 sensor...
✓ BME680 sensor initialized successfully
  - Temperature oversampling: 1x
  - Pressure oversampling: 1x
  - Humidity oversampling: 1x
  - Gas sensor: Disabled

IoT Prototype System Ready (USART4)
Type 'start' to begin
> start
System started! Type 'help' for available commands.
> test sensor
Testing BME680 sensor (USART4)...
Test successful!
Temperature: 23.45°C
Pressure: 101325.00 Pa
Humidity: 45.67%
```

### Error Example (Sensor Not Available):
```
========================================
IoT Prototype System - STM32G071RB
========================================
System Clock: 16 MHz
I2C1 Configuration: PB8 (SCL), PB9 (SDA)
USART2: PA2 (TX), PA3 (RX) - 115200 baud
USART4: PA0 (TX), PA1 (RX) - 115200 baud
LED Status: PA5
========================================

Checking BME680 sensor presence...
✗ BME680 sensor not found on I2C bus
Troubleshooting steps:
  1. Check I2C connections:
     - PB8 (SCL) → BME680 SCL
     - PB9 (SDA) → BME680 SDA
  2. Verify power supply:
     - BME680 VCC → 3.3V
     - BME680 GND → GND
  3. Check pull-up resistors (4.7kΩ recommended)
  4. Verify I2C address (default: 0x76)
System will continue without sensor functionality

IoT Prototype System Ready (USART2)
Type 'start' to begin
> start
System started! Type 'help' for available commands.
> read temperature
Error: BME680 sensor not available
```

## Technical Details

### I2C Configuration
- I2C1 configured at 100kHz
- 7-bit addressing
- BME680 I2C address: 0x76 (default)

### UART Configuration
- USART2: 115200 baud, 8N1 (PA2/PA3)
- USART4: 115200 baud, 8N1 (PA0/PA1)
- Both UARTs operate independently with separate command sessions

### Sensor Configuration
- Temperature oversampling: 1x
- Pressure oversampling: 1x
- Humidity oversampling: 1x
- Filter: Off
- Gas sensor: Disabled (for simplicity)

## Future Enhancements
- LoRa HAT integration (SPI communication)
- Gas resistance measurement
- Data logging functionality
- Wireless communication
- Web interface
- Multiple sensor support

## Troubleshooting

### Common Issues
1. **Sensor not detected**: Check I2C connections and power supply
2. **Communication errors**: Verify UART settings and connections
3. **Invalid readings**: Ensure proper sensor initialization

### Debug Information
- LED on PA5 blinks to indicate system is running
- UART debug messages show initialization status
- Error messages are displayed for failed operations

## Dependencies
- STM32 HAL library
- Bosch BME680 sensor library
- Standard C library functions

## License
This project is part of an educational IoT prototype system. 