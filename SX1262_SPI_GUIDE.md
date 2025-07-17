# SX1262 LoRa Module SPI Configuration Guide

## Hardware Connections

Connect your SX1262 LoRa HAT to the STM32 Nucleo board using the following pin mapping:

| SX1262 HAT Pin  | Function        | STM32 Pin           | Description                    |
| --------------- | --------------- | ------------------- | ------------------------------ |
| GPIO11 (Pin 23) | SCK             | PA5  (pin 11) cn 10 | SPI Clock                      |
| GPIO10 (Pin 19) | MOSI            | PA7  (pin 11) cn 15 |  SPI Master Out Slave In        |
| GPIO9 (Pin 21)  | MISO            | PA6  (pin 13) cn 10 |  SPI Master In Slave Out        |
| GPIO8 (Pin 24)  | NSS / CS        | PA4  (pin 32) cn 07 | SPI Chip Select                |
| GPIO25 (Pin 22) | RESET           | PC0  (pin 18) cn 10 | Module Reset (Active Low)      |
| GPIO24 (Pin 18) | BUSY / DIO1     | PA1  (pin 16) cn 10 | Busy Signal (Input)            |
| 3.3V (Pin 1)    | Power           | 3.3V on Nucleo      | Power Supply                   |
| GND (Pin 6)     | Ground          | GND                 | Common Ground                   |

## STM32CubeIDE Configuration

### SPI1 Configuration
- **Mode**: Master
- **Direction**: 2 Lines Full-Duplex
- **Data Size**: 8 Bits
- **Clock Polarity**: Low
- **Clock Phase**: 1 Edge
- **NSS**: Hardware NSS Output Signal
- **Baud Rate Prescaler**: 2 (8.0 MBits/s)

### GPIO Configuration
- **PA4**: SPI1_NSS (Hardware NSS Output)
- **PA5**: SPI1_SCK (SPI Clock)
- **PA6**: SPI1_MISO (SPI MISO)
- **PA7**: SPI1_MOSI (SPI MOSI)
- **PC0**: GPIO_Output (SX1262 RESET)
- **PC1**: GPIO_Input (SX1262 BUSY)

## Software Implementation

### Key Features

1. **SPI Communication**: Direct SPI interface with SX1262
2. **BUSY Polling**: Automatic BUSY pin monitoring
3. **Hardware Reset**: Controlled reset via PC0
4. **LoRa Configuration**: Complete SX1262 LoRa setup
5. **Error Handling**: Comprehensive error checking

### SX1262 Driver Functions

#### Core Functions
- `sx1262_init()` - Initialize SX1262 module
- `sx1262_reset()` - Hardware reset of SX1262
- `sx1262_configure_lora()` - Configure LoRa parameters

#### SPI Communication
- `sx1262_spi_read()` - Read data from SX1262
- `sx1262_spi_write()` - Write data to SX1262
- `sx1262_spi_write_read()` - Write-read operation
- `sx1262_wait_for_busy()` - Wait for BUSY pin to go low

#### LoRa Operations
- `sx1262_send_message()` - Send LoRa packet
- `sx1262_send_sensor_data()` - Send sensor data via LoRa
- `sx1262_test_transmission()` - Test LoRa transmission
- `sx1262_get_status()` - Get module status
- `sx1262_print_config()` - Print configuration

### LoRa Configuration Parameters

```c
#define SX1262_FREQUENCY_HZ        868000000  // 868 MHz (EU band)
#define SX1262_TX_POWER_DBM        14         // 14 dBm output power
#define SX1262_SPREADING_FACTOR    7          // SF7
#define SX1262_BANDWIDTH           125        // 125 kHz
#define SX1262_CODING_RATE         1          // 4/5
#define SX1262_PREAMBLE_LENGTH     8
#define SX1262_PAYLOAD_LENGTH      64
#define SX1262_SYNC_WORD           0x12
```

## Usage Commands

### Available Commands
- `lb` - Broadcast sensor data via SX1262 LoRa
- `lc` - Show SX1262 LoRa configuration
- `lt` - Test SX1262 LoRa transmission
- `ls` - Scan for LoRa signals (5s)
- `lm` - Start continuous monitoring
- `lst` - Stop LoRa monitoring
- `lr` - Get current RSSI

### Example Usage
```
> start
System started! Type 'help' for available commands.
> lt
Testing SX1262 LoRa transmission...
✓ SX1262 transmission test successful

> lb
✓ Sensor data sent successfully via SX1262 LoRa

> lc
=== SX1262 LoRa Configuration ===
Status: Module detected
Initialized: Yes
Frequency: 868 MHz (EU band)
Spreading Factor: SF7
Bandwidth: 125 kHz
Coding Rate: 4/5
TX Power: 14 dBm
Sync Word: 0x12
Payload Length: 64 bytes
Preamble Length: 8 symbols
CRC: Enabled
IQ Inversion: Disabled
========================
```

## Troubleshooting

### Common Issues

1. **Module Not Detected**
   - Check power supply (3.3V)
   - Verify RESET pin connection
   - Check BUSY pin connection

2. **SPI Communication Errors**
   - Verify SPI pin connections
   - Check SPI configuration in STM32CubeIDE
   - Ensure proper NSS timing

3. **Transmission Failures**
   - Check antenna connection
   - Verify frequency configuration
   - Monitor BUSY pin during transmission

4. **BUSY Timeout**
   - Check PC1 connection to SX1262 BUSY pin
   - Verify module power supply
   - Check for hardware faults

### Debug Information

The system provides detailed debug output via USART2 and USART4:
- Initialization status
- SPI communication errors
- LoRa configuration status
- Transmission results

## Hardware Setup Tips

1. **Power Supply**: Ensure stable 3.3V supply
2. **Ground Connection**: Common ground between STM32 and SX1262
3. **Antenna**: Connect proper antenna for 868 MHz
4. **Pull-up Resistors**: May be needed for SPI lines if not already present
5. **Shielding**: Consider RF shielding for better performance

## Performance Characteristics

- **SPI Speed**: 8.0 MBits/s
- **LoRa Frequency**: 868 MHz (EU band)
- **TX Power**: 14 dBm
- **Spreading Factor**: SF7
- **Bandwidth**: 125 kHz
- **Coding Rate**: 4/5
- **Payload Size**: Up to 64 bytes
- **Preamble**: 8 symbols

## Future Enhancements

1. **Interrupt Support**: Add DIO1 interrupt handling
2. **Receive Mode**: Implement packet reception
3. **LoRaWAN**: Add LoRaWAN protocol support
4. **Power Management**: Implement sleep modes
5. **Frequency Hopping**: Add frequency hopping support 
