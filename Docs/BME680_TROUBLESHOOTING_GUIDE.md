# BME680 Sensor Troubleshooting Guide

## Quick Diagnostic Steps

1. **Connect to the serial terminal** (USART2 or USART4 at 115200 baud)
2. **Run the comprehensive diagnostic**: Type `bme diagnostic` or `bd`
3. **Check the output** and follow the specific recommendations below

## Common Issues and Solutions

### 1. No I2C Devices Found
**Symptoms**: Diagnostic shows "No I2C devices found"
**Solutions**:
- Check wiring connections:
  - PA9 (SCL) → BME680 SCL
  - PA10 (SDA) → BME680 SDA
  - 3.3V → BME680 VCC
  - GND → BME680 GND
- Add 4.7kΩ pull-up resistors to SCL and SDA lines
- Verify power supply is stable at 3.3V
- Check for loose connections or cold solder joints

### 2. Wrong Chip ID
**Symptoms**: Device responds but chip ID is not 0x61
**Solutions**:
- Verify you have a genuine BME680 sensor (not BME280 or other)
- Check if the sensor is damaged or counterfeit
- Try the alternative I2C address (0x77) if available

### 3. Sensor Initialization Fails
**Symptoms**: Chip ID correct but initialization returns error
**Solutions**:
- Check power supply stability
- Verify I2C timing (try slower I2C speed)
- Ensure sensor is not in sleep mode
- Check for electromagnetic interference

### 4. Sensor Reads Zero Values
**Symptoms**: Initialization succeeds but all readings are 0.0
**Solutions**:
- Check sensor operation mode (should be in forced mode)
- Verify measurement timing
- Check if sensor is actually measuring (raw ADC values)
- Ensure proper delay between measurements

### 5. Invalid Values (NaN)
**Symptoms**: Readings show NaN or invalid values
**Solutions**:
- Check library compatibility
- Verify calibration data is valid
- Check for memory corruption
- Ensure proper data conversion

## Advanced Diagnostic Commands

Run these commands in sequence to isolate the issue:

1. **`i2c scan`** - Scan for I2C devices
2. **`bme diagnostic`** - Run comprehensive diagnostic
3. **`raw registers`** - Read raw sensor registers
4. **`calib data`** - Check calibration data
5. **`test sensor`** - Test sensor readings

## Hardware Checklist

### Wiring Verification
- [ ] SCL (PA9) → BME680 SCL
- [ ] SDA (PA10) → BME680 SDA  
- [ ] 3.3V → BME680 VCC
- [ ] GND → BME680 GND
- [ ] 4.7kΩ pull-up resistors on SCL and SDA

### Power Supply
- [ ] 3.3V supply is stable
- [ ] No voltage drops under load
- [ ] Clean power supply (no noise)
- [ ] Adequate current capacity

### Physical Connections
- [ ] All connections are secure
- [ ] No cold solder joints
- [ ] Proper wire gauge
- [ ] Short wire lengths
- [ ] No electromagnetic interference

## Software Configuration

### I2C Settings
- Clock speed: 100kHz (standard mode)
- 7-bit addressing
- No clock stretching
- Pull-up resistors enabled

### Sensor Configuration
- Temperature oversampling: 1x
- Pressure oversampling: 1x  
- Humidity oversampling: 1x
- Gas sensor: Disabled
- Filter: Off
- Operation mode: Forced

## Expected Behavior

### Normal Operation
1. I2C scan finds device at 0x76 or 0x77
2. Chip ID reads 0x61
3. Initialization succeeds (returns 0)
4. Raw ADC values are non-zero
5. Calibration data is valid
6. Temperature readings are reasonable (15-35°C)
7. Pressure readings are reasonable (900-1100 hPa)
8. Humidity readings are reasonable (20-80%)

### Error Codes
- `BME68X_OK` (0): Success
- `BME68X_E_NULL_PTR` (-1): Null pointer
- `BME68X_E_COM_FAIL` (-2): Communication failure
- `BME68X_E_INVALID_LENGTH` (-3): Invalid length
- `BME68X_E_DEV_NOT_FOUND` (-4): Device not found
- `BME68X_E_INVALID_ADDR` (-5): Invalid address

## Contact Information

If the issue persists after following this guide:
1. Document the exact error messages
2. Note the diagnostic output
3. Check the hardware connections again
4. Consider replacing the sensor if all else fails

## Additional Resources

- BME680 Datasheet
- STM32G0 Reference Manual
- I2C Protocol Specification
- BME68X Library Documentation 