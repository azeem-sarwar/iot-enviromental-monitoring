# USART3 Wiring Guide & Troubleshooting

## Hardware Configuration

### STM32G071RB USART3 Pin Assignment
```
STM32G071RB Pin    →    USB-to-UART Converter / PuTTY
─────────────────────────────────────────────────────
PC10 (USART3_TX)   →    RX (Receive)
PC11 (USART3_RX)   →    TX (Transmit)
GND               →    GND (Ground)
```

### Detailed Wiring Diagram
```
┌─────────────────────────────────────────────────────────────┐
│                    STM32G071RB                              │
│  ┌─────────────────────────────────────────────────────┐    │
│  │                                                     │    │
│  │  PC10 ──────────────────────────────────────────────┐ │    │
│  │  (USART3_TX)                                      │ │    │
│  │                                                   │ │    │
│  │  PC11 ─────────────────────────────────────────────┤ │    │
│  │  (USART3_RX)                                     │ │    │
│  │                                                  │ │    │
│  │  GND ─────────────────────────────────────────────┤ │    │
│  └─────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                                │
                                │
                                ▼
                    ┌─────────────────────────┐
                    │   USB-to-UART Converter │
                    │   (CH340, CP2102, etc.) │
                    │                         │
                    │  RX  ←─────────────────┘
                    │  TX  ←─────────────────┘
                    │  GND ←─────────────────┘
                    └─────────────────────────┘
                                │
                                │ USB Cable
                                ▼
                    ┌─────────────────────────┐
                    │        Computer         │
                    │      (PuTTY/Serial)     │
                    └─────────────────────────┘
```

## Current Configuration Analysis

### Software Configuration (Correct)
- **Baud Rate:** 115200
- **Data Bits:** 8
- **Stop Bits:** 1
- **Parity:** None
- **Flow Control:** None
- **Alternate Function:** GPIO_AF4_USART3

### GPIO Configuration (Correct)
- **PC10:** USART3_TX (Alternate Function 4)
- **PC11:** USART3_RX (Alternate Function 4)
- **Configuration:** GPIO_MODE_AF_PP (Alternate Function Push-Pull)

## Troubleshooting Guide

### Issue 1: No Communication on USART3
**Symptoms:**
- No response in PuTTY
- No data received
- System appears to hang

**Possible Causes & Solutions:**

1. **Wiring Issues**
   - **Problem:** Incorrect TX/RX connection
   - **Solution:** 
     - STM32 PC10 (TX) → USB Converter RX
     - STM32 PC11 (RX) → USB Converter TX
     - **Remember:** TX from one device goes to RX of the other

2. **USB-to-UART Converter Issues**
   - **Problem:** Wrong driver or converter not recognized
   - **Solution:**
     - Install correct USB-to-UART driver (CH340, CP2102, etc.)
     - Check Device Manager for COM port assignment
     - Try different USB cable

3. **PuTTY Configuration Issues**
   - **Problem:** Wrong COM port or settings
   - **Solution:**
     - Select correct COM port in PuTTY
     - Set baud rate to 115200
     - Set data bits to 8
     - Set stop bits to 1
     - Set parity to None
     - Set flow control to None

### Issue 2: Garbled or Corrupted Data
**Symptoms:**
- Random characters in PuTTY
- Incomplete messages
- Wrong data received

**Possible Causes & Solutions:**

1. **Baud Rate Mismatch**
   - **Problem:** Different baud rates between STM32 and PuTTY
   - **Solution:** Ensure both use 115200 baud

2. **Voltage Level Issues**
   - **Problem:** 3.3V vs 5V logic levels
   - **Solution:** 
     - Use 3.3V USB-to-UART converter
     - Or use level shifter if converter is 5V

3. **Noise or Interference**
   - **Problem:** Electrical noise affecting signals
   - **Solution:**
     - Keep wires short
     - Add decoupling capacitors
     - Use shielded cable

### Issue 3: One-Way Communication
**Symptoms:**
- Can send from STM32 to PuTTY but not receive
- Or can send from PuTTY to STM32 but not receive

**Possible Causes & Solutions:**

1. **TX/RX Connection Reversed**
   - **Problem:** Wrong pin connections
   - **Solution:** Swap TX and RX connections

2. **Software Configuration**
   - **Problem:** UART not properly initialized
   - **Solution:** Check UART initialization code

## Testing Steps

### Step 1: Hardware Verification
1. **Check Connections:**
   ```
   STM32 PC10 (TX) → USB Converter RX
   STM32 PC11 (RX) → USB Converter TX
   STM32 GND → USB Converter GND
   ```

2. **Check USB Converter:**
   - LED should blink when data is transmitted
   - Device should appear in Device Manager

### Step 2: Software Verification
1. **Check PuTTY Settings:**
   ```
   Connection type: Serial
   Serial line: COMx (check Device Manager)
   Speed: 115200
   Data bits: 8
   Stop bits: 1
   Parity: None
   Flow control: None
   ```

2. **Test Basic Communication:**
   - Open PuTTY
   - Press Enter to see if you get a response
   - Try sending commands like "help" or "ls"

### Step 3: Debug Testing
1. **Send Test Message:**
   ```
   ls    # Should show LoRa status
   rt    # Should show temperature
   help  # Should show available commands
   ```

2. **Check for System Messages:**
   - Look for initialization messages
   - Check for error messages
   - Verify command responses

## Common USB-to-UART Converters

### CH340/CH341
- **Driver:** CH341SER.EXE
- **Voltage:** 5V (may need level shifter)
- **Common Issues:** Driver installation problems

### CP2102
- **Driver:** CP210x_Windows_Drivers
- **Voltage:** 3.3V (compatible with STM32)
- **Common Issues:** None

### FT232RL
- **Driver:** FTDI drivers
- **Voltage:** 5V (may need level shifter)
- **Common Issues:** Driver installation

### PL2303
- **Driver:** PL2303_Prolific_DriverInstaller
- **Voltage:** 5V (may need level shifter)
- **Common Issues:** Driver compatibility

## Expected Behavior

### System Startup Messages
When the system boots, you should see:
```
========================================
IoT Prototype System - STM32G071RB
========================================
System Clock: 16 MHz
I2C1 Configuration: PA9 (SCL), PC110 (SDA)
USART2: PA2 (TX), PA3 (RX) - 115200 baud
USART3: PC10 (TX), PC11 (RX) - 115200 baud
SPI1: PA5 (SCK), PA6 (MISO), PA7 (MOSI)
LoRa: PA4 (NSS), PC0 (RESET)
LED Status: PA5
========================================
```

### Command Responses
```
> ls
LoRa Status: Initialized
> rt
Temperature: 25.6°C
> help
Available commands:
  ls - LoRa status
  lt - LoRa test transmission
  ls "message" - Send LoRa message
  ls 5000 - Scan LoRa signals (5s)
  lm - Start LoRa monitoring
  lst - Stop LoRa monitoring
  lr - Get LoRa RSSI
  rt - Read temperature
  rp - Read pressure
  rh - Read humidity
  help - Show this help
```

## Quick Fix Checklist

- [ ] Check TX/RX connections (PC10→RX, PC11→TX)
- [ ] Verify USB converter is recognized in Device Manager
- [ ] Install correct USB-to-UART driver
- [ ] Set PuTTY to correct COM port and 115200 baud
- [ ] Check for voltage level compatibility (3.3V vs 5V)
- [ ] Verify STM32 is powered and running
- [ ] Check for any error messages in PuTTY
- [ ] Try different USB cable
- [ ] Test with different USB-to-UART converter

## Advanced Debugging

### Using Oscilloscope
- **TX Signal:** Should show 115200 baud square wave
- **RX Signal:** Should show data when typing in PuTTY
- **Voltage Levels:** Should be 3.3V for STM32

### Using Logic Analyzer
- **Protocol:** UART
- **Baud Rate:** 115200
- **Data Format:** 8N1 (8 data bits, no parity, 1 stop bit)

### Using Multimeter
- **Voltage Check:** Measure voltage on PC10 and PC11
- **Continuity Test:** Check wire connections
- **Ground Check:** Verify GND connection 