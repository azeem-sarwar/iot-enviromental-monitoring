# Wiring Configuration

This document describes the hardware wiring configuration for the STM32G071RB project.

---

## MCU: STM32G071RB

### 1. **SX1262 LoRa Module**
| SX1262 Pin | STM32 Pin | Port | Notes                |
|------------|-----------|------|----------------------|
| NSS        | PA4       | GPIOA| SPI1_NSS             |
| SCK        | PA5       | GPIOA| SPI1_SCK             |
| MISO       | PA6       | GPIOA| SPI1_MISO            |
| MOSI       | PA7       | GPIOA| SPI1_MOSI            |
| RESET      | PC0       | GPIOC| LoRa Reset           |
| BUSY       | PC3       | GPIOC| LoRa Busy            |
| DIO1       | (optional)|      | LoRa IRQ (not used)  |

### 2. **USART2 (Debug Interface)**
| Function | STM32 Pin | Port | Notes         |
|----------|-----------|------|---------------|
| TX       | PA2       | GPIOA| USART2_TX     |
| RX       | PA3       | GPIOA| USART2_RX     |
| Baudrate | 115200    |      | 8N1           |

### 3. **USART3 (Clean Interface)**
| Function | STM32 Pin | Port | Notes         |
|----------|-----------|------|---------------|
| TX       | PC10      | GPIOC| USART3_TX     |
| RX       | PC11      | GPIOC| USART3_RX     |
| Baudrate | 115200    |      | 8N1           |

### 4. **BME680 Sensor (I2C)**
| BME680 Pin | STM32 Pin | Port | Notes      |
|------------|-----------|------|------------|
| SDA        | PB7       | GPIOB| I2C1_SDA   |
| SCL        | PB6       | GPIOB| I2C1_SCL   |
| VCC        | 3.3V      |      | Power      |
| GND        | GND       |      | Ground     |

### 5. **LED (Status Indicator)**
| Function | STM32 Pin | Port | Notes      |
|----------|-----------|------|------------|
| LED      | PC1       | GPIOC| Status LED |

---

**Note:** Ensure all grounds (GND) are connected together between STM32, SX1262, and BME680 for proper operation. 