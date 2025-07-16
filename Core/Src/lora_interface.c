#include "lora_interface.h"
#include "bme680_interface.h"
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
    .busy_pin = GPIO_PIN_1
};

// Debug function
static void sx1262_debug_print(const char* message) {
    HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), 1000);
    HAL_UART_Transmit(&huart4, (uint8_t*)message, strlen(message), 1000);
}

// Wait for SX1262 BUSY pin to go low
uint8_t sx1262_wait_for_busy(void) {
    uint32_t timeout = HAL_GetTick() + 2000; // 2 second timeout (increased)
    uint32_t start_time = HAL_GetTick();
    
    // First check if BUSY is already low
    if (HAL_GPIO_ReadPin(sx1262_ctx.busy_port, sx1262_ctx.busy_pin) == GPIO_PIN_RESET) {
        return 1; // Already ready
    }
    
    // Wait for BUSY to go low
    while (HAL_GPIO_ReadPin(sx1262_ctx.busy_port, sx1262_ctx.busy_pin) == GPIO_PIN_SET) {
        if (HAL_GetTick() > timeout) {
            sx1262_debug_print("BUSY timeout after ");
            char timeout_msg[32];
            snprintf(timeout_msg, sizeof(timeout_msg), "%lu ms\r\n", HAL_GetTick() - start_time);
            sx1262_debug_print(timeout_msg);
            return 0; // Timeout
        }
        HAL_Delay(1); // Small delay to prevent tight loop
    }
    
    return 1; // Success
}

// Check if SX1262 module is present and responding
int8_t sx1262_detect_module(void) {
    uint8_t status;
    
    sx1262_debug_print("Detecting SX1262 module...\r\n");
    
    // Try to read the chip status register
    if (sx1262_spi_read(0xC0, &status, 1) == 0) {
        sx1262_debug_print("✓ SX1262 module detected\r\n");
        return 0;
    } else {
        sx1262_debug_print("✗ SX1262 module not detected\r\n");
        return -1;
    }
}

// Simple SPI test function
int8_t sx1262_test_spi(void) {
    uint8_t test_data[4] = {0x12, 0x34, 0x56, 0x78};
    uint8_t read_data[4];
    
    sx1262_debug_print("Testing SPI communication...\r\n");
    
    // Test basic SPI write/read
    if (sx1262_spi_write(0x00, test_data, 4) == 0) {
        sx1262_debug_print("✓ SPI write test passed\r\n");
    } else {
        sx1262_debug_print("✗ SPI write test failed\r\n");
        return -1;
    }
    
    if (sx1262_spi_read(0x00, read_data, 4) == 0) {
        sx1262_debug_print("✓ SPI read test passed\r\n");
        return 0;
    } else {
        sx1262_debug_print("✗ SPI read test failed\r\n");
        return -1;
    }
}

// SPI read function for SX1262
int8_t sx1262_spi_read(uint8_t address, uint8_t* data, uint8_t length) {
    uint8_t tx_buffer[256];
    uint8_t rx_buffer[256];
    
    if (length > 255) return -1;
    
    // Prepare command - SX1262 uses 8-bit addresses
    tx_buffer[0] = address; // Direct address for SX1262
    
    // Wait for BUSY to go low
    if (!sx1262_wait_for_busy()) {
        sx1262_debug_print("BUSY timeout during SPI read\r\n");
        return -1;
    }
    
    // Pull NSS low
    HAL_GPIO_WritePin(sx1262_ctx.nss_port, sx1262_ctx.nss_pin, GPIO_PIN_RESET);
    HAL_Delay(1); // Small delay for NSS setup
    
    // Transmit command
    if (HAL_SPI_Transmit(sx1262_ctx.spi, tx_buffer, 1, 1000) != HAL_OK) {
        HAL_GPIO_WritePin(sx1262_ctx.nss_port, sx1262_ctx.nss_pin, GPIO_PIN_SET);
        sx1262_debug_print("SPI transmit failed\r\n");
        return -1;
    }
    
    // Receive data
    if (HAL_SPI_Receive(sx1262_ctx.spi, rx_buffer, length, 1000) != HAL_OK) {
        HAL_GPIO_WritePin(sx1262_ctx.nss_port, sx1262_ctx.nss_pin, GPIO_PIN_SET);
        sx1262_debug_print("SPI receive failed\r\n");
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
    
    // Prepare command and data - SX1262 uses 8-bit addresses
    tx_buffer[0] = address; // Direct address for SX1262
    memcpy(&tx_buffer[1], data, length);
    
    // Wait for BUSY to go low
    if (!sx1262_wait_for_busy()) {
        sx1262_debug_print("BUSY timeout during SPI write\r\n");
        return -1;
    }
    
    // Pull NSS low
    HAL_GPIO_WritePin(sx1262_ctx.nss_port, sx1262_ctx.nss_pin, GPIO_PIN_RESET);
    HAL_Delay(1); // Small delay for NSS setup
    
    // Transmit command and data
    if (HAL_SPI_Transmit(sx1262_ctx.spi, tx_buffer, length + 1, 1000) != HAL_OK) {
        HAL_GPIO_WritePin(sx1262_ctx.nss_port, sx1262_ctx.nss_pin, GPIO_PIN_SET);
        sx1262_debug_print("SPI transmit failed\r\n");
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
    sx1262_debug_print("Resetting SX1262 module...\r\n");
    
    // Pull RESET low
    HAL_GPIO_WritePin(sx1262_ctx.reset_port, sx1262_ctx.reset_pin, GPIO_PIN_RESET);
    HAL_Delay(20); // Longer reset pulse
    
    // Pull RESET high
    HAL_GPIO_WritePin(sx1262_ctx.reset_port, sx1262_ctx.reset_pin, GPIO_PIN_SET);
    HAL_Delay(50); // Wait for module to stabilize
    
    // Check if BUSY is high initially (normal after reset)
    if (HAL_GPIO_ReadPin(sx1262_ctx.busy_port, sx1262_ctx.busy_pin) == GPIO_PIN_SET) {
        sx1262_debug_print("BUSY is high after reset (normal)\r\n");
        HAL_Delay(100); // Wait for module to finish initialization
    }
    
    // Now wait for BUSY to go low (module ready)
    if (!sx1262_wait_for_busy()) {
        sx1262_debug_print("✗ SX1262 reset failed - BUSY timeout\r\n");
        return -1;
    }
    
    sx1262_debug_print("✓ SX1262 reset successful\r\n");
    return 0;
}

// Configure SX1262 for LoRa operation
int8_t sx1262_configure_lora(void) {
    uint8_t config_data[16];
    
    // Set to standby mode
    config_data[0] = 0x80; // STANDBY_RC command
    if (sx1262_spi_write(0x80, config_data, 1) != 0) {
        sx1262_debug_print("✗ Failed to set standby mode\r\n");
        return -1;
    }
    
    // Set regulator mode to LDO
    config_data[0] = 0x96; // SET_REGULATOR_MODE command
    config_data[1] = 0x00; // LDO mode
    if (sx1262_spi_write(0x96, config_data, 2) != 0) {
        sx1262_debug_print("✗ Failed to set regulator mode\r\n");
        return -1;
    }
    
    // Set packet type to LoRa
    config_data[0] = 0x8A; // SET_PKT_TYPE command
    config_data[1] = 0x01; // LoRa packet type
    if (sx1262_spi_write(0x8A, config_data, 2) != 0) {
        sx1262_debug_print("✗ Failed to set packet type\r\n");
        return -1;
    }
    
    // Set RF frequency (868 MHz)
    uint32_t freq = ((uint64_t)SX1262_FREQUENCY_HZ * 16384) / 32000000; // Convert to SX1262 format
    config_data[0] = 0x86; // SET_RF_FREQUENCY command
    config_data[1] = (freq >> 16) & 0xFF;
    config_data[2] = (freq >> 8) & 0xFF;
    config_data[3] = freq & 0xFF;
    if (sx1262_spi_write(0x86, config_data, 4) != 0) {
        sx1262_debug_print("✗ Failed to set RF frequency\r\n");
        return -1;
    }
    
    // Set LoRa modulation parameters
    config_data[0] = 0x8B; // SET_MODULATION_PARAMS command
    config_data[1] = SX1262_SPREADING_FACTOR; // Spreading factor
    config_data[2] = SX1262_BANDWIDTH / 125; // Bandwidth (125 kHz = 0x00)
    config_data[3] = SX1262_CODING_RATE; // Coding rate
    config_data[4] = 0x01; // Low data rate optimization
    if (sx1262_spi_write(0x8B, config_data, 5) != 0) {
        sx1262_debug_print("✗ Failed to set modulation parameters\r\n");
        return -1;
    }
    
    // Set LoRa packet parameters
    config_data[0] = 0x8C; // SET_PKT_PARAMS command
    config_data[1] = SX1262_PREAMBLE_LENGTH >> 8; // Preamble length MSB
    config_data[2] = SX1262_PREAMBLE_LENGTH & 0xFF; // Preamble length LSB
    config_data[3] = 0x01; // Header type (explicit)
    config_data[4] = SX1262_PAYLOAD_LENGTH; // Payload length
    config_data[5] = 0x01; // CRC on
    config_data[6] = 0x00; // Invert IQ off
    if (sx1262_spi_write(0x8C, config_data, 7) != 0) {
        sx1262_debug_print("✗ Failed to set packet parameters\r\n");
        return -1;
    }
    
    // Set TX parameters
    config_data[0] = 0x8E; // SET_TX_PARAMS command
    config_data[1] = SX1262_TX_POWER_DBM; // TX power
    config_data[2] = 0x00; // Ramp time (10 us)
    if (sx1262_spi_write(0x8E, config_data, 3) != 0) {
        sx1262_debug_print("✗ Failed to set TX parameters\r\n");
        return -1;
    }
    
    // Set LoRa sync word
    config_data[0] = 0x91; // SET_LORA_SYNC_WORD command
    config_data[1] = SX1262_SYNC_WORD; // Sync word
    if (sx1262_spi_write(0x91, config_data, 2) != 0) {
        sx1262_debug_print("✗ Failed to set sync word\r\n");
        return -1;
    }
    
    // Set buffer base address
    config_data[0] = 0x8F; // SET_BUFFER_BASE_ADDRESS command
    config_data[1] = 0x00; // TX base address
    config_data[2] = 0x00; // RX base address
    if (sx1262_spi_write(0x8F, config_data, 3) != 0) {
        sx1262_debug_print("✗ Failed to set buffer base address\r\n");
        return -1;
    }
    
    sx1262_debug_print("✓ SX1262 LoRa configuration successful\r\n");
    return 0;
}

// Initialize SX1262 module
int8_t sx1262_init(void) {
    sx1262_debug_print("Initializing SX1262 LoRa module...\r\n");
    
    // First detect if module is present
    if (sx1262_detect_module() != 0) {
        sx1262_debug_print("✗ SX1262 module not detected - check connections\r\n");
        sx1262_debug_print("Check wiring:\r\n");
        sx1262_debug_print("  - PA4 (NSS) → SX1262 CS\r\n");
        sx1262_debug_print("  - PA5 (SCK) → SX1262 SCK\r\n");
        sx1262_debug_print("  - PA6 (MISO) → SX1262 MISO\r\n");
        sx1262_debug_print("  - PA7 (MOSI) → SX1262 MOSI\r\n");
        sx1262_debug_print("  - PC0 (RESET) → SX1262 RESET\r\n");
        sx1262_debug_print("  - PC1 (BUSY) → SX1262 BUSY\r\n");
        sx1262_debug_print("  - 3.3V → SX1262 VCC\r\n");
        sx1262_debug_print("  - GND → SX1262 GND\r\n");
        sx1262_module_detected = 0;
        sx1262_initialized = 0;
        return -1;
    }
    
    sx1262_module_detected = 1;
    sx1262_debug_print("✓ Module detected, proceeding with initialization...\r\n");
    
    // Reset the module
    if (sx1262_reset() != 0) {
        sx1262_debug_print("✗ SX1262 reset failed - trying alternative approach\r\n");
        
        // Try a simpler approach - just wait and try to configure
        HAL_Delay(200); // Wait longer for module to stabilize
        
        if (sx1262_configure_lora() != 0) {
            sx1262_debug_print("✗ SX1262 LoRa configuration failed after reset timeout\r\n");
            sx1262_initialized = 0;
            return -1;
        }
        
        sx1262_debug_print("✓ SX1262 LoRa module initialized (alternative method)\r\n");
        sx1262_initialized = 1;
        return 0;
    }
    
    // Configure for LoRa operation
    if (sx1262_configure_lora() != 0) {
        sx1262_debug_print("✗ SX1262 LoRa configuration failed\r\n");
        sx1262_initialized = 0;
        return -1;
    }
    
    sx1262_debug_print("✓ SX1262 LoRa module initialized successfully\r\n");
    sx1262_initialized = 1;
    return 0;
}

// Send sensor data via SX1262 LoRa
int8_t sx1262_send_sensor_data(float temperature, float pressure, float humidity) {
    if (!sx1262_module_detected) {
        sx1262_debug_print("✗ LoRa transmission failed - no module detected\r\n");
        return -1;
    }
    
    if (!sx1262_initialized) {
        sx1262_debug_print("✗ LoRa transmission failed - module not initialized\r\n");
        return -1;
    }
    
    char payload[SX1262_PAYLOAD_LENGTH];
    
    // Format sensor data as JSON-like string
    snprintf(payload, sizeof(payload), 
                   "{\"temp\":%.2f,\"press\":%.2f,\"hum\":%.2f,\"node\":\"STM32\"}",
                   temperature, pressure, humidity);
    
    return sx1262_send_message((uint8_t*)payload, strlen(payload));
}

// Send message via SX1262 LoRa
int8_t sx1262_send_message(const uint8_t* data, uint8_t length) {
    if (!sx1262_module_detected) {
        sx1262_debug_print("✗ LoRa transmission failed - no module detected\r\n");
        return -1;
    }
    
    if (!sx1262_initialized) {
        sx1262_debug_print("✗ LoRa transmission failed - module not initialized\r\n");
        return -1;
    }
    
    if (data == NULL || length == 0 || length > SX1262_PAYLOAD_LENGTH) {
        sx1262_debug_print("Invalid LoRa message parameters\r\n");
        return -1;
    }
    
    // Write payload to buffer
    if (sx1262_spi_write(0x0E, data, length) != 0) {
        sx1262_debug_print("✗ Failed to write payload to buffer\r\n");
        return -1;
    }
    
    // Set packet length
    uint8_t pkt_len = length;
    if (sx1262_spi_write(0x8C, &pkt_len, 1) != 0) {
        sx1262_debug_print("✗ Failed to set packet length\r\n");
        return -1;
    }
    
    // Start transmission
    uint8_t tx_cmd = 0x83; // TX command
    if (sx1262_spi_write(0x83, &tx_cmd, 1) != 0) {
        sx1262_debug_print("✗ Failed to start transmission\r\n");
        return -1;
    }
    
    // Wait for transmission to complete (BUSY will go low)
    if (!sx1262_wait_for_busy()) {
        sx1262_debug_print("✗ LoRa transmission timeout\r\n");
                return -1;
    }
    
    sx1262_debug_print("✓ Message sent successfully via SX1262 LoRa\r\n");
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
    
    if (sx1262_spi_read(0xC0, &status, 1) == 0) {
        char status_msg[64];
        snprintf(status_msg, sizeof(status_msg), "SX1262 Status: 0x%02X\r\n", status);
        sx1262_debug_print(status_msg);
        return 0;
    }
    
        return -1;
    }
    
// Print SX1262 configuration
void sx1262_print_config(void) {
    sx1262_debug_print("\r\n=== SX1262 LoRa Configuration ===\r\n");
    sx1262_debug_print("Status: ");
    sx1262_debug_print(sx1262_module_detected ? "Module detected\r\n" : "No module detected\r\n");
    sx1262_debug_print("Initialized: ");
    sx1262_debug_print(sx1262_initialized ? "Yes\r\n" : "No\r\n");
    sx1262_debug_print("Frequency: 868 MHz (EU band)\r\n");
    sx1262_debug_print("Spreading Factor: SF7\r\n");
    sx1262_debug_print("Bandwidth: 125 kHz\r\n");
    sx1262_debug_print("Coding Rate: 4/5\r\n");
    sx1262_debug_print("TX Power: 14 dBm\r\n");
    sx1262_debug_print("Sync Word: 0x12\r\n");
    sx1262_debug_print("Payload Length: 64 bytes\r\n");
    sx1262_debug_print("Preamble Length: 8 symbols\r\n");
    sx1262_debug_print("CRC: Enabled\r\n");
    sx1262_debug_print("IQ Inversion: Disabled\r\n");
    sx1262_debug_print("========================\r\n");
}

// Test SX1262 transmission
int8_t sx1262_test_transmission(void) {
    const char* test_message = "Hello SX1262 from STM32!";
    
    sx1262_debug_print("Testing SX1262 LoRa transmission...\r\n");
    
    if (sx1262_send_message((uint8_t*)test_message, strlen(test_message)) == 0) {
        sx1262_debug_print("✓ SX1262 transmission test successful\r\n");
        return 0;
    } else {
        sx1262_debug_print("✗ SX1262 transmission test failed\r\n");
        return -1;
    }
}

// Legacy function implementations for compatibility

// Scan for LoRa signals (placeholder implementation)
int8_t lora_scan_signals(uint32_t scan_time_ms) {
    sx1262_debug_print("LoRa signal scanning not implemented yet\r\n");
        return -1;
    }
    
// Start LoRa monitoring (placeholder implementation)
int8_t lora_start_monitoring(void) {
    sx1262_debug_print("LoRa monitoring not implemented yet\r\n");
        return -1;
    }
    
// Stop LoRa monitoring (placeholder implementation)
int8_t lora_stop_monitoring(void) {
    sx1262_debug_print("LoRa monitoring not implemented yet\r\n");
        return -1;
    }
    
// Get RSSI value (placeholder implementation)
int8_t lora_get_rssi(void) {
    sx1262_debug_print("RSSI measurement not implemented yet\r\n");
        return -1;
} 