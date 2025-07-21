# SX1262 LoRa Module SPI Connection Guide

## Updated Pin Configuration (PC0/PC3)

| SX1262 HAT Pin  | Function        | STM32 Pin           | Description                    |
| --------------- | --------------- | ------------------- | ------------------------------ |
| GPIO11 (Pin 23) | SCK             | PA5  (pin 11) cn 10 | SPI Clock                      |
| GPIO10 (Pin 19) | MOSI            | PA7  (pin 15) cn 15 |  SPI Master Out Slave In        |
| GPIO9 (Pin 21)  | MISO            | PA6  (pin 13) cn 10 |  SPI Master In Slave Out        |
| GPIO8 (Pin 24)  | NSS / CS        | PA4  (pin 32) cn 07 | SPI Chip Select                |
| GPIO25 (Pin 22) | RESET           | PC0  (pin 18) cn 10 | Module Reset (Active Low)      |
| GPIO24 (Pin 18) | BUSY / DIO1     | PC3  (pin 16) cn 10 | Busy Signal (Input)            |
| 3.3V (Pin 1)    | Power           | 3.3V on Nucleo      | Power Supply                   |
| GND (Pin 6)     | Ground          | GND                 | Common Ground                   |

## Connection Details

### SPI Interface
- **SCK (PA5)**: SPI clock signal
- **MOSI (PA7)**: Data from STM32 to SX1262
- **MISO (PA6)**: Data from SX1262 to STM32
- **NSS/CS (PA4)**: Chip select (active low)

### Control Signals
- **RESET (PC0)**: Module reset (active low, pulled high normally)
- **BUSY (PC3)**: Busy signal (input to STM32, indicates module is busy)

### Power
- **3.3V**: Power supply for SX1262 module
- **GND**: Common ground connection

## Important Notes

1. **PC0 (RESET)**: Configured as output in the code
2. **PC3 (BUSY)**: Configured as input with interrupt capability
3. **SPI Configuration**: 
   - Mode: Master
   - Data Size: 8-bit
   - Clock Polarity: Low
   - Clock Phase: 1 edge
   - Baud Rate: Prescaler 2 (8 MHz)

## Troubleshooting

### If SX1262 is not detected:
1. Check all SPI connections (SCK, MOSI, MISO, NSS)
2. Verify RESET (PC0) is properly connected
3. Check BUSY (PC3) connection
4. Ensure 3.3V power supply is stable
5. Verify ground connection

### If transmission fails:
1. Check BUSY pin behavior during transmission
2. Verify SPI timing and clock frequency
3. Check antenna connection
4. Verify frequency configuration (868 MHz for EU)

## Code Configuration

The code has been updated to use:
- **PC0** for RESET
- **PC3** for BUSY

All GPIO configurations and SX1262 context structures have been updated accordingly. 
