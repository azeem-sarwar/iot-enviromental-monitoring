#include "lora/lora_interface.h"
#include "bme680/bme680_interface.h"
#include <string.h>
#include <stdio.h>

// External handles
extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart4;

// SX1262 module status
static uint8_t sx1262_module_detected = 0;
static uint8_t sx1262_initialized = 0;

// SX1262 context
static sx1262_context_t sx1262_ctx = {
    .spi = &hspi1,
    .nss_port = GPIOA,
    .nss_pin = GPIO_PIN_4,
    .reset_port = GPIOC,
    .reset_pin = GPIO_PIN_0,
    .busy_port = GPIOC,
    .busy_pin = GPIO_PIN_3
};

// SX1262 Command definitions (from datasheet)
#define SX1262_CMD_NOP                    0x00
#define SX1262_CMD_GET_STATUS             0xC0
#define SX1262_CMD_WRITE_REGISTER         0x0D
#define SX1262_CMD_READ_REGISTER          0x1D
#define SX1262_CMD_WRITE_BUFFER           0x0E
#define SX1262_CMD_READ_BUFFER            0x1E
#define SX1262_CMD_SET_SLEEP              0x84
#define SX1262_CMD_SET_STANDBY            0x80
#define SX1262_CMD_SET_FS                 0xC1
#define SX1262_CMD_SET_TX                 0x83
#define SX1262_CMD_SET_RX                 0x82
#define SX1262_CMD_SET_STOP_TIMER_ON_PREAMBLE 0x9F
#define SX1262_CMD_SET_RX_DUTY_CYCLE     0x94
#define SX1262_CMD_SET_CAD                0xC5
#define SX1262_CMD_SET_TX_CONTINUOUS_WAVE 0xD1
#define SX1262_CMD_SET_TX_INFINITE_PREAMBLE 0xD2
#define SX1262_CMD_SET_REGULATOR_MODE    0x96
#define SX1262_CMD_CALIBRATE             0x89
#define SX1262_CMD_CALIBRATE_IMAGE       0x98
#define SX1262_CMD_SET_PA_CFG            0x95
#define SX1262_CMD_SET_RX_TX_FALLBACK_MODE 0x93
#define SX1262_CMD_SET_DIO_IRQ_PARAMS    0x8D
#define SX1262_CMD_SET_RF_FREQUENCY      0x86
#define SX1262_CMD_SET_PKT_TYPE          0x8A
#define SX1262_CMD_GET_PKT_STATUS        0x11
#define SX1262_CMD_SET_MODULATION_PARAMS 0x8B
#define SX1262_CMD_SET_PKT_PARAMS        0x8C
#define SX1262_CMD_SET_TX_PARAMS         0x8E
#define SX1262_CMD_SET_BUFFER_BASE_ADDRESS 0x8F
#define SX1262_CMD_SET_LORA_SYNC_WORD   0x91
#define SX1262_CMD_SET_LORA_SYMB_NUM_TIMEOUT 0x0A
#define SX1262_CMD_SET_LORA_PKT_TYPE    0x0B
#define SX1262_CMD_GET_RSSI_INST         0x15
#define SX1262_CMD_SET_GF_MODE           0x92

// SX1262 Register addresses
#define SX1262_REG_LORA_SYNC_WORD_MSB    0x0740
#define SX1262_REG_LORA_SYNC_WORD_LSB    0x0741
#define SX1262_REG_RANDOM_NUMBER_GEN     0x0819
#define SX1262_REG_ANALOG_DFO            0x081A
#define SX1262_REG_ANALOG_DFO_MSB        0x081B
#define SX1262_REG_ANALOG_DFO_LSB        0x081C

// Wait for SX1262 BUSY pin to go low
uint8_t sx1262_wait_for_busy(void) {
    uint32_t timeout = HAL_GetTick() + 2000; // 2 second timeout
    
    // First check if BUSY is already low
    if (HAL_GPIO_ReadPin(sx1262_ctx.busy_port, sx1262_ctx.busy_pin) == GPIO_PIN_RESET) {
        return 1; // Already ready
    }
    
    // Wait for BUSY to go low
    while (HAL_GPIO_ReadPin(sx1262_ctx.busy_port, sx1262_ctx.busy_pin) == GPIO_PIN_SET) {
        if (HAL_GetTick() > timeout) {
            return 0; // Timeout
        }
        HAL_Delay(1); // Small delay to prevent tight loop
    }
    
    return 1; // Success
}

// Check if SX1262 module is present and responding
int8_t sx1262_detect_module(void) {
    uint8_t status;
    
    // Try to read the chip status register
    if (sx1262_spi_read(SX1262_CMD_GET_STATUS, &status, 1) == 0) {
        return 0;
    } else {
        return -1;
    }
}

// Simple SPI test function
int8_t sx1262_test_spi(void) {
    uint8_t test_data[4] = {0x12, 0x34, 0x56, 0x78};
    uint8_t read_data[4];
    
    // Test basic SPI write/read
    if (sx1262_spi_write(SX1262_CMD_WRITE_BUFFER, test_data, 4) == 0) {
        if (sx1262_spi_read(SX1262_CMD_READ_BUFFER, read_data, 4) == 0) {
            return 0;
        }
    }
    
    return -1;
}

// SPI read function for SX1262
int8_t sx1262_spi_read(uint8_t address, uint8_t* data, uint8_t length) {
    uint8_t tx_buffer[256];
    uint8_t rx_buffer[256];
    
    if (length > 255) return -1;
    
    // Prepare command - SX1262 uses command codes
    tx_buffer[0] = address;
    
    // Wait for BUSY to go low
    if (!sx1262_wait_for_busy()) {
        return -1;
    }
    
    // Pull NSS low
    HAL_GPIO_WritePin(sx1262_ctx.nss_port, sx1262_ctx.nss_pin, GPIO_PIN_RESET);
    HAL_Delay(1); // Small delay for NSS setup
    
    // Transmit command
    if (HAL_SPI_Transmit(sx1262_ctx.spi, tx_buffer, 1, 1000) != HAL_OK) {
        HAL_GPIO_WritePin(sx1262_ctx.nss_port, sx1262_ctx.nss_pin, GPIO_PIN_SET);
        return -1;
    }
    
    // Receive data
    if (HAL_SPI_Receive(sx1262_ctx.spi, rx_buffer, length, 1000) != HAL_OK) {
        HAL_GPIO_WritePin(sx1262_ctx.nss_port, sx1262_ctx.nss_pin, GPIO_PIN_SET);
        return -1;
    }
    
    // Pull NSS high
    HAL_GPIO_WritePin(sx1262_ctx.nss_port, sx1262_ctx.nss_pin, GPIO_PIN_SET);
    
    // Copy received data
    memcpy(data, rx_buffer, length);
    
    return 0;
}

// SPI write function for SX1262
int8_t sx1262_spi_write(uint8_t address, const uint8_t* data, uint8_t length) {
    uint8_t tx_buffer[256];
    
    if (length > 255) return -1;
    
    // Prepare command and data - SX1262 uses command codes
    tx_buffer[0] = address;
    memcpy(&tx_buffer[1], data, length);
    
    // Wait for BUSY to go low
    if (!sx1262_wait_for_busy()) {
        return -1;
    }
    
    // Pull NSS low
    HAL_GPIO_WritePin(sx1262_ctx.nss_port, sx1262_ctx.nss_pin, GPIO_PIN_RESET);
    HAL_Delay(1); // Small delay for NSS setup
    
    // Transmit command and data
    if (HAL_SPI_Transmit(sx1262_ctx.spi, tx_buffer, length + 1, 1000) != HAL_OK) {
        HAL_GPIO_WritePin(sx1262_ctx.nss_port, sx1262_ctx.nss_pin, GPIO_PIN_SET);
        return -1;
    }
    
    // Pull NSS high
    HAL_GPIO_WritePin(sx1262_ctx.nss_port, sx1262_ctx.nss_pin, GPIO_PIN_SET);
    
    return 0;
}

// SPI write-read function for SX1262
int8_t sx1262_spi_write_read(uint8_t address, const uint8_t* tx_data, uint8_t* rx_data, uint8_t length) {
    uint8_t tx_buffer[256];
    uint8_t rx_buffer[256];
    
    if (length > 255) return -1;
    
    // Prepare command and data
    tx_buffer[0] = address & 0x7F; // Read command
    memcpy(&tx_buffer[1], tx_data, length);
    
    // Wait for BUSY to go low
    if (!sx1262_wait_for_busy()) {
        return -1;
    }
    
    // Pull NSS low
    HAL_GPIO_WritePin(sx1262_ctx.nss_port, sx1262_ctx.nss_pin, GPIO_PIN_RESET);
    
    // Transmit and receive
    if (HAL_SPI_TransmitReceive(sx1262_ctx.spi, tx_buffer, rx_buffer, length + 1, 1000) != HAL_OK) {
        HAL_GPIO_WritePin(sx1262_ctx.nss_port, sx1262_ctx.nss_pin, GPIO_PIN_SET);
        return -1;
    }
    
    // Pull NSS high
    HAL_GPIO_WritePin(sx1262_ctx.nss_port, sx1262_ctx.nss_pin, GPIO_PIN_SET);
    
    // Copy received data (skip first byte which is the command)
    memcpy(rx_data, &rx_buffer[1], length);
    
    return 0;
}

// Reset SX1262 module
int8_t sx1262_reset(void) {
    // Pull RESET low
    HAL_GPIO_WritePin(sx1262_ctx.reset_port, sx1262_ctx.reset_pin, GPIO_PIN_RESET);
    HAL_Delay(20); // Longer reset pulse
    
    // Pull RESET high
    HAL_GPIO_WritePin(sx1262_ctx.reset_port, sx1262_ctx.reset_pin, GPIO_PIN_SET);
    HAL_Delay(50); // Wait for module to stabilize
    
    // Check if BUSY is high initially (normal after reset)
    if (HAL_GPIO_ReadPin(sx1262_ctx.busy_port, sx1262_ctx.busy_pin) == GPIO_PIN_SET) {
        HAL_Delay(100); // Wait for module to finish initialization
    }
    
    // Now wait for BUSY to go low (module ready)
    if (!sx1262_wait_for_busy()) {
        return -1;
    }
    
    return 0;
}

// Configure SX1262 for LoRa operation following datasheet sequence
int8_t sx1262_configure_lora(void) {
    uint8_t config_data[16];
    
    // Step 1: Set to standby mode
    config_data[0] = 0x00; // STANDBY_RC mode
    if (sx1262_spi_write(SX1262_CMD_SET_STANDBY, config_data, 1) != 0) {
        return -1;
    }
    
    // Step 2: Set regulator mode to LDO
    config_data[0] = 0x00; // LDO mode
    if (sx1262_spi_write(SX1262_CMD_SET_REGULATOR_MODE, config_data, 1) != 0) {
        return -1;
    }
    
    // Step 3: Set packet type to LoRa
    config_data[0] = 0x01; // LoRa packet type
    if (sx1262_spi_write(SX1262_CMD_SET_PKT_TYPE, config_data, 1) != 0) {
        return -1;
    }
    
    // Step 4: Set RF frequency (868 MHz)
    // Frequency calculation: Freq = (Freq_Hz * 2^25) / 32MHz
    uint32_t freq = ((uint64_t)SX1262_FREQUENCY_HZ * 33554432) / 32000000;
    config_data[0] = (freq >> 16) & 0xFF;
    config_data[1] = (freq >> 8) & 0xFF;
    config_data[2] = freq & 0xFF;
    if (sx1262_spi_write(SX1262_CMD_SET_RF_FREQUENCY, config_data, 3) != 0) {
        return -1;
    }
    
    // Step 5: Set LoRa modulation parameters
    config_data[0] = SX1262_SPREADING_FACTOR; // Spreading factor (SF7)
    config_data[1] = 0x00; // Bandwidth 125 kHz (0x00 = 125kHz)
    config_data[2] = SX1262_CODING_RATE; // Coding rate (4/5)
    config_data[3] = 0x01; // Low data rate optimization enabled
    if (sx1262_spi_write(SX1262_CMD_SET_MODULATION_PARAMS, config_data, 4) != 0) {
        return -1;
    }
    
    // Step 6: Set LoRa packet parameters
    config_data[0] = (SX1262_PREAMBLE_LENGTH >> 8) & 0xFF; // Preamble length MSB
    config_data[1] = SX1262_PREAMBLE_LENGTH & 0xFF; // Preamble length LSB
    config_data[2] = 0x01; // Header type (explicit)
    config_data[3] = SX1262_PAYLOAD_LENGTH; // Payload length
    config_data[4] = 0x01; // CRC on
    config_data[5] = 0x00; // Invert IQ off
    if (sx1262_spi_write(SX1262_CMD_SET_PKT_PARAMS, config_data, 6) != 0) {
        return -1;
    }
    
    // Step 7: Set TX parameters
    config_data[0] = SX1262_TX_POWER_DBM; // TX power in dBm
    config_data[1] = 0x00; // Ramp time (10 us)
    if (sx1262_spi_write(SX1262_CMD_SET_TX_PARAMS, config_data, 2) != 0) {
        return -1;
    }
    
    // Step 8: Set buffer base address
    config_data[0] = 0x00; // TX base address
    config_data[1] = 0x00; // RX base address
    if (sx1262_spi_write(SX1262_CMD_SET_BUFFER_BASE_ADDRESS, config_data, 2) != 0) {
        return -1;
    }
    
    // Step 9: Set LoRa sync word
    config_data[0] = SX1262_SYNC_WORD; // Sync word
    if (sx1262_spi_write(SX1262_CMD_SET_LORA_SYNC_WORD, config_data, 1) != 0) {
        return -1;
    }
    
    // Step 10: Configure DIO1 for TX_DONE interrupt
    config_data[0] = 0x01; // IRQ_TX_DONE
    config_data[1] = 0x00; // IRQ_RX_DONE
    config_data[2] = 0x00; // IRQ_PREAMBLE_DETECTED
    config_data[3] = 0x00; // IRQ_SYNC_WORD_VALID
    config_data[4] = 0x00; // IRQ_HEADER_VALID
    config_data[5] = 0x00; // IRQ_HEADER_ERR
    config_data[6] = 0x00; // IRQ_CRC_ERR
    config_data[7] = 0x00; // IRQ_CAD_DONE
    config_data[8] = 0x00; // IRQ_CAD_DETECTED
    config_data[9] = 0x00; // IRQ_TIMEOUT
    if (sx1262_spi_write(SX1262_CMD_SET_DIO_IRQ_PARAMS, config_data, 10) != 0) {
        return -1;
    }
    
    return 0;
}

// Initialize SX1262 module
int8_t sx1262_init(void) {
    // First detect if module is present
    if (sx1262_detect_module() != 0) {
        sx1262_module_detected = 0;
        sx1262_initialized = 0;
        return -1;
    }
    
    sx1262_module_detected = 1;
    
    // Reset the module
    if (sx1262_reset() != 0) {
        // Try a simpler approach - just wait and try to configure
        HAL_Delay(200); // Wait longer for module to stabilize
        
        if (sx1262_configure_lora() != 0) {
            sx1262_initialized = 0;
            return -1;
        }
        
        sx1262_initialized = 1;
        return 0;
    }
    
    // Configure for LoRa operation
    if (sx1262_configure_lora() != 0) {
        sx1262_initialized = 0;
        return -1;
    }
    
    sx1262_initialized = 1;
    return 0;
}

// Send sensor data via SX1262 LoRa
int8_t sx1262_send_sensor_data(float temperature, float pressure, float humidity) {
    if (!sx1262_module_detected) {
        return -1;
    }
    
    if (!sx1262_initialized) {
        return -1;
    }
    
    char payload[SX1262_PAYLOAD_LENGTH];
    
    // Format sensor data as JSON-like string
    snprintf(payload, sizeof(payload), 
                   "{\"temp\":%.2f,\"press\":%.2f,\"hum\":%.2f,\"node\":\"STM32\"}",
                   temperature, pressure, humidity);
    
    return sx1262_send_message((uint8_t*)payload, strlen(payload));
}

// Send message via SX1262 LoRa following datasheet TX sequence
int8_t sx1262_send_message(const uint8_t* data, uint8_t length) {
    if (!sx1262_module_detected) {
        return -1;
    }
    
    if (!sx1262_initialized) {
        return -1;
    }
    
    if (data == NULL || length == 0 || length > SX1262_PAYLOAD_LENGTH) {
        return -1;
    }
    
    // Step 1: Set to standby mode
    uint8_t standby_cmd = 0x00; // STANDBY_RC mode
    if (sx1262_spi_write(SX1262_CMD_SET_STANDBY, &standby_cmd, 1) != 0) {
        return -1;
    }
    
    // Step 2: Write payload to buffer
    if (sx1262_spi_write(SX1262_CMD_WRITE_BUFFER, data, length) != 0) {
        return -1;
    }
    
    // Step 3: Set packet length (update packet parameters)
    uint8_t pkt_params[6];
    pkt_params[0] = (SX1262_PREAMBLE_LENGTH >> 8) & 0xFF; // Preamble length MSB
    pkt_params[1] = SX1262_PREAMBLE_LENGTH & 0xFF; // Preamble length LSB
    pkt_params[2] = 0x01; // Header type (explicit)
    pkt_params[3] = length; // Payload length (dynamic)
    pkt_params[4] = 0x01; // CRC on
    pkt_params[5] = 0x00; // Invert IQ off
    if (sx1262_spi_write(SX1262_CMD_SET_PKT_PARAMS, pkt_params, 6) != 0) {
        return -1;
    }
    
    // Step 4: Start transmission with timeout
    uint8_t tx_cmd = 0x00; // TX command with timeout
    uint8_t timeout_ms = 1000; // 1 second timeout
    uint8_t tx_params[2] = {tx_cmd, timeout_ms};
    if (sx1262_spi_write(SX1262_CMD_SET_TX, tx_params, 2) != 0) {
        return -1;
    }
    
    // Step 5: Wait for transmission to complete (BUSY will go low)
    uint32_t tx_start = HAL_GetTick();
    uint32_t tx_timeout = tx_start + 2000; // 2 second timeout
    
    while (HAL_GPIO_ReadPin(sx1262_ctx.busy_port, sx1262_ctx.busy_pin) == GPIO_PIN_SET) {
        if (HAL_GetTick() > tx_timeout) {
            return -1;
        }
        HAL_Delay(1);
    }
    
    // Step 6: Check packet status (optional)
    uint8_t pkt_status;
    sx1262_spi_read(SX1262_CMD_GET_PKT_STATUS, &pkt_status, 1);
    
    return 0;
}

// Process SX1262 interrupts
void sx1262_process_irq(void) {
    // For now, we use polling for BUSY pin
    // This function can be extended for DIO1 interrupt handling
}

// Get SX1262 status
int8_t sx1262_get_status(void) {
    uint8_t status;
    
    if (sx1262_spi_read(SX1262_CMD_GET_STATUS, &status, 1) == 0) {
        return 0;
    }
    
    return -1;
}

// Print SX1262 configuration
void sx1262_print_config(void) {
    // Configuration is handled by command interface
}

// Test SX1262 transmission
int8_t sx1262_test_transmission(void) {
    const char* test_message = "Hello SX1262 from STM32!";
    
    if (sx1262_send_message((uint8_t*)test_message, strlen(test_message)) == 0) {
        return 0;
    } else {
        return -1;
    }
}

// Legacy function implementations for compatibility

// Scan for LoRa signals (placeholder implementation)
int8_t lora_scan_signals(uint32_t scan_time_ms) {
    return -1;
}

// Start LoRa monitoring (placeholder implementation)
int8_t lora_start_monitoring(void) {
    return -1;
}

// Stop LoRa monitoring (placeholder implementation)
int8_t lora_stop_monitoring(void) {
    return -1;
}

// Get RSSI value (placeholder implementation)
int8_t lora_get_rssi(void) {
    return -1;
} 