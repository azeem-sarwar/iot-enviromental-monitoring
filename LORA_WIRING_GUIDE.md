# LoRa Module Wiring Guide & Troubleshooting

## Hardware Configuration

### STM32G071RB Pin Assignment
```
STM32G071RB Pin    →    LoRa Module (SX126x) Pin
─────────────────────────────────────────────────
PA5 (SCK)         →    SCK (Clock)
PA6 (MISO)        →    MISO (Data In)
PA7 (MOSI)        →    MOSI (Data Out)
PA4 (NSS/CS)      →    CEO (NSS/CS (Chip Select))
PC0 (RESET)       →    IDSC (RESET )
PC2 (DIO1)        →    P2 (DIO1 (Interrupt))  
3.3V              →    VCC
GND               →    GND
```

### Detailed Wiring Diagram
```
┌─────────────────────────────────────────────────────────────┐
│                    STM32G071RB                              │
│  ┌─────────────────────────────────────────────────────┐    │
│  │                                                     │    │
│  │  PA5 ──────────────────────────────────────────────┐ │    │
│  │  (SCK)                                            │ │    │
│  │                                                   │ │    │
│  │  PA6 ─────────────────────────────────────────────┤ │    │
│  │  (MISO)                                          │ │    │
│  │                                                  │ │    │
│  │  PA7 ─────────────────────────────────────────────┤ │    │
│  │  (MOSI)                                         │ │    │
│  │                                                 │ │    │
│  │  PA4 ─────────────────────────────────────────────┤ │    │
│  │  (NSS)                                         │ │    │
│  │                                                │ │    │
│  │  PC0 ─────────────────────────────────────────────┤ │    │
│  │  (RESET)                                      │ │    │
│  │                                               │ │    │
│  │  PC1 ─────────────────────────────────────────────┤ │    │
│  │  (DIO1)                                     │ │    │
│  │                                              │ │    │
│  │  3.3V ─────────────────────────────────────────────┤ │    │
│  │                                              │ │    │
│  │  GND ──────────────────────────────────────────────┤ │    │
│  └─────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                                │
                                │
                                ▼
                    ┌─────────────────────────┐
                    │    LoRa Module          │
                    │    (SX126x)             │
                    │                         │
                    │  SCK  ←─────────────────┘
                    │  MISO ←─────────────────┘
                    │  MOSI ←─────────────────┘
                    │  NSS  ←─────────────────┘
                    │  RESET ←────────────────┘
                    │  DIO1 ←─────────────────┘
                    │  VCC  ←─────────────────┘
                    │  GND  ←─────────────────┘
                    └─────────────────────────┘
```

## Performance Optimizations Applied

### 1. SPI Speed Improvement
- **Before:** SPI_BAUDRATEPRESCALER_8 (2 MHz at 16 MHz system clock)
- **After:** SPI_BAUDRATEPRESCALER_2 (8 MHz at 16 MHz system clock)
- **Improvement:** 4x faster SPI communication

### 2. Timeout Optimization
- **Before:** 1000ms timeout for each SPI operation
- **After:** 100ms timeout for each SPI operation
- **Improvement:** 10x faster error detection

### 3. Interrupt Support
- **Added:** DIO1 pin configuration for interrupt-driven operation
- **Benefit:** Non-blocking LoRa operations, better responsiveness

## Troubleshooting Guide

### Issue 1: LoRa Module Not Detected
**Symptoms:**
- "✗ LoRa initialization failed - no module detected"
- No response from LoRa commands

**Possible Causes & Solutions:**

1. **Power Supply Issues**
   - Check if LoRa module receives 3.3V power
   - Verify GND connection
   - Measure voltage at LoRa VCC pin (should be 3.3V ±0.1V)

2. **SPI Connection Issues**
   - Verify all SPI pins are connected correctly
   - Check for loose connections or cold solder joints
   - Ensure no short circuits between pins

3. **Reset Pin Issues**
   - Verify PC0 (RESET) is connected to LoRa RESET pin
   - Check if reset pin is floating (should be pulled up to 3.3V)

4. **NSS Pin Issues**
   - Verify PA4 (NSS) is connected to LoRa NSS/CS pin
   - Check if NSS pin is properly controlled by software

### Issue 2: Slow or Unresponsive LoRa Operations
**Symptoms:**
- Commands take a long time to respond
- Timeout errors
- System appears to hang during LoRa operations

**Possible Causes & Solutions:**

1. **SPI Speed Issues**
   - Verify SPI_BAUDRATEPRESCALER_2 is set in code
   - Check if system clock is running at 16 MHz
   - Ensure SPI pins are configured for high speed

2. **Long Wires or Poor Connections**
   - Keep SPI wires as short as possible (< 10cm recommended)
   - Use proper wire gauge (22-24 AWG)
   - Check for loose connections

3. **Power Supply Noise**
   - Add 100nF ceramic capacitor between LoRa VCC and GND
   - Use separate power supply for LoRa if possible
   - Add ferrite bead on power line

4. **Interference Issues**
   - Keep SPI wires away from high-frequency signals
   - Use twisted pair or shielded cable for SPI
   - Add pull-up resistors (4.7kΩ) on SPI lines if needed

### Issue 3: Intermittent Communication
**Symptoms:**
- LoRa works sometimes but fails randomly
- Occasional timeout errors
- Data corruption

**Possible Causes & Solutions:**

1. **Timing Issues**
   - Add small delays between SPI operations
   - Ensure proper NSS timing (select before command, deselect after)
   - Check if interrupts are interfering with SPI

2. **Voltage Level Issues**
   - Verify 3.3V logic levels are compatible
   - Check for voltage drops under load
   - Ensure proper ground connection

3. **EMI/EMC Issues**
   - Add decoupling capacitors
   - Use proper PCB layout with ground plane
   - Keep high-frequency signals away from sensitive circuits

## Testing Commands

### Basic Testing
1. **Check LoRa Status:**
   ```
   ls
   ```

2. **Test Transmission:**
   ```
   lt
   ```

3. **Send Custom Message:**
   ```
   ls "Hello LoRa"
   ```

4. **Scan for Signals:**
   ```
   ls 5000
   ```

5. **Start Monitoring:**
   ```
   lm
   ```

6. **Stop Monitoring:**
   ```
   lst
   ```

7. **Get RSSI:**
   ```
   lr
   ```

### Performance Testing
1. **Measure Response Time:**
   - Send `ls` command and measure time to response
   - Should be < 100ms for basic commands

2. **Test Transmission Speed:**
   - Send multiple messages in sequence
   - Check for timeout errors

3. **Monitor Interrupts:**
   - Watch for DIO1 interrupt activity
   - Verify interrupt handler is called

## Expected Performance

### With Optimizations:
- **SPI Speed:** 8 MHz
- **Command Response Time:** < 100ms
- **Transmission Time:** < 500ms per message
- **Interrupt Latency:** < 1ms

### Without Optimizations:
- **SPI Speed:** 2 MHz
- **Command Response Time:** 200-500ms
- **Transmission Time:** 1-2 seconds per message
- **Interrupt Latency:** Not available (polling mode)

## Additional Recommendations

1. **Hardware Improvements:**
   - Use proper PCB with ground plane
   - Add decoupling capacitors (100nF + 10µF)
   - Use ferrite beads on power lines
   - Keep wires short and properly routed

2. **Software Improvements:**
   - Implement error recovery mechanisms
   - Add retry logic for failed operations
   - Use interrupt-driven operation for better performance
   - Implement proper state machine for LoRa operations

3. **Debugging Tools:**
   - Use oscilloscope to check SPI signals
   - Monitor power supply with multimeter
   - Use logic analyzer for timing analysis
   - Add debug LEDs for status indication 
