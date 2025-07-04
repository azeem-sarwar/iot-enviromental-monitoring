# IoT Prototype System - STM32G071RB with BME680

## Project Overview
This is an IoT prototype system based on the STM32G071RB Nucleo board that collects environmental data from a BME680 sensor and provides a command interface via UART.

## Hardware Setup
- **MCU**: STM32G071RB Nucleo board
- **Sensor**: BME680 (Temperature, Pressure, Humidity, Gas)
- **Connections**:
  - PB8 → I2C1_SCL → BME680 SCL
  - PB9 → I2C1_SDA → BME680 SDA
  - USART2 → USB-UART adapter for PC communication
  - USART4 → Additional UART interface

## Features Implemented

### 1. BME680 Sensor Interface
- I2C communication with BME680 sensor
- Temperature, pressure, and humidity reading
- Sensor initialization and configuration
- Error handling and status reporting

### 2. Command Interface System
- UART-based command processing
- Interactive command prompt
- Real-time command parsing and execution
- Help system with available commands

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

2. Connect USB-UART adapter to USART2:
   - TX → PA2
   - RX → PA3

### 2. Software Setup
1. Compile and flash the project to STM32G071RB
2. Open a terminal program (PuTTY, Tera Term, etc.)
3. Configure terminal:
   - Baud rate: 115200
   - Data bits: 8
   - Stop bits: 1
   - Parity: None
   - Flow control: None

### 3. System Operation
1. Power on the system
2. Wait for initialization messages
3. Type `start` to begin command interface
4. Use `help` to see available commands
5. Execute sensor readings and mathematical operations

## Example Session
```
IoT Prototype System Ready
Type 'start' to begin
BME680 sensor initialized successfully
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
> test sensor
Testing BME680 sensor...
Test successful!
Temperature: 23.45°C
Pressure: 101325.00 Pa
Humidity: 45.67%
```

## Technical Details

### I2C Configuration
- I2C1 configured at 100kHz
- 7-bit addressing
- BME680 I2C address: 0x76 (default)

### UART Configuration
- USART2: 115200 baud, 8N1
- USART4: 115200 baud, 8N1

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