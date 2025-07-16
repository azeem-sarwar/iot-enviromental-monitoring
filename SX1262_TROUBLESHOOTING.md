# SX1262 LoRa Module Troubleshooting Guide

## Issue: LoRa Module Not Detected

If your SX1262 LoRa module is not being detected, follow these troubleshooting steps:

### 1. **Hardware Connection Verification**

**Check all connections with a multimeter:**

| STM32 Pin | SX1262 Pin | Function | Expected Voltage |
|-----------|------------|----------|------------------|
| PA4       | GPIO8 (24) | NSS/CS   | 3.3V (idle)     |
| PA5       | GPIO11 (23)| SCK      | 0-3.3V (clock)  |
| PA6       | GPIO9 (21) | MISO     | 0-3.3V (data)   |
| PA7       | GPIO10 (19)| MOSI     | 0-3.3V (data)   |
| PC0       | GPIO25 (22)| RESET    | 3.3V (idle)     |
| PC1       | GPIO24 (18)| BUSY     | 0-3.3V (status) |
| 3.3V      | Pin 1      | VCC      | 3.3V            |
| GND       | Pin 6      | GND      | 0V              |

### 2. **Power Supply Check**

- **Voltage**: Ensure exactly 3.3V (not 5V!)
- **Current**: SX1262 needs ~50mA during transmission
- **Stability**: Use a stable power supply
- **Ground**: Common ground between STM32 and SX1262

### 3. **SPI Configuration Verification**

**Check STM32CubeIDE SPI1 settings:**
- Mode: Master
- Direction: 2 Lines Full-Duplex
- Data Size: 8 Bits
- Clock Polarity: Low
- Clock Phase: 1 Edge
- NSS: Hardware NSS Output Signal
- Baud Rate: 8.0 MBits/s

### 4. **Software Debugging Steps**

#### Step 1: Test Basic SPI Communication
```
> tspi
```
This will test basic SPI read/write operations.

#### Step 2: Check Module Detection
```
> lt
```
This will attempt to initialize the SX1262 and show detailed error messages.

#### Step 3: Monitor Debug Output
Watch for these messages:
- "Detecting SX1262 module..."
- "BUSY timeout during SPI read"
- "SPI transmit failed"
- "SPI receive failed"

### 5. **Common Issues and Solutions**

#### Issue 1: "BUSY timeout during SPI read"
**Cause**: SX1262 not responding or BUSY pin not connected
**Solutions**:
- Check PC1 (BUSY) connection
- Verify SX1262 power supply
- Check RESET pin connection

#### Issue 2: "SPI transmit failed"
**Cause**: SPI communication issues
**Solutions**:
- Check all SPI pin connections
- Verify SPI configuration in STM32CubeIDE
- Check for short circuits or open circuits

#### Issue 3: "SX1262 module not detected"
**Cause**: Module not responding to SPI commands
**Solutions**:
- Verify power supply (3.3V, not 5V!)
- Check all pin connections
- Ensure proper ground connection
- Try hardware reset (cycle power)

### 6. **Hardware Reset Procedure**

1. **Power Cycle**: Disconnect power, wait 10 seconds, reconnect
2. **Manual Reset**: Pull PC0 (RESET) low for 100ms, then release
3. **Check BUSY**: PC1 should go low after reset

### 7. **Advanced Debugging**

#### Check SPI Signals with Oscilloscope:
- **SCK**: Should show clock pulses during communication
- **MOSI**: Should show data during write operations
- **MISO**: Should show data during read operations
- **NSS**: Should pulse low during SPI transactions
- **BUSY**: Should be low when module is ready

#### Expected SPI Timing:
- NSS setup time: >100ns
- Clock frequency: 8 MHz max
- Data valid: 50ns before/after clock edge

### 8. **Module-Specific Checks**

#### SX1262 HAT Specific:
- Check if HAT is properly seated
- Verify pin mapping matches your HAT
- Check for bent pins or poor connections
- Ensure proper orientation

#### Power Supply Requirements:
- **VCC**: 3.3V ±0.1V
- **Current**: 50mA typical, 120mA peak
- **Decoupling**: 100nF ceramic capacitor near VCC pin

### 9. **Software Debugging Commands**

```
> tspi          # Test basic SPI communication
> lt            # Test LoRa transmission
> lc            # Show LoRa configuration
> help          # Show all available commands
```

### 10. **Expected Debug Output**

**Successful Detection:**
```
Detecting SX1262 module...
✓ SX1262 module detected
✓ SX1262 reset successful
✓ SX1262 LoRa configuration successful
✓ SX1262 LoRa module initialized successfully
```

**Failed Detection:**
```
Detecting SX1262 module...
✗ SX1262 module not detected - check connections
Check wiring:
  - PA4 (NSS) → SX1262 CS
  - PA5 (SCK) → SX1262 SCK
  - PA6 (MISO) → SX1262 MISO
  - PA7 (MOSI) → SX1262 MOSI
  - PC0 (RESET) → SX1262 RESET
  - PC1 (BUSY) → SX1262 BUSY
  - 3.3V → SX1262 VCC
  - GND → SX1262 GND
```

### 11. **Final Checklist**

- [ ] All pins connected correctly
- [ ] Power supply is 3.3V (not 5V!)
- [ ] Common ground between STM32 and SX1262
- [ ] SPI configuration correct in STM32CubeIDE
- [ ] No short circuits or open circuits
- [ ] SX1262 HAT properly seated
- [ ] Hardware reset performed
- [ ] Debug output monitored

### 12. **Contact Information**

If issues persist after following this guide:
1. Double-check all connections
2. Try a different SX1262 module if available
3. Verify STM32 board functionality
4. Check for any hardware damage

**Remember**: The most common issue is incorrect power supply voltage (5V instead of 3.3V) or poor ground connections. 