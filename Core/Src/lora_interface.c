#include "lora_interface.h"
#include "bme680_interface.h"
#include "sx126x.h"
#include <string.h>
#include <stdio.h>

// External handles
extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart4;

// LoRa module status
static uint8_t lora_module_detected = 0;
static uint8_t lora_initialized = 0;

// SX126x configuration
static sx126x_mod_params_lora_t lora_mod_params = {
    .sf = SX126X_LORA_SF7,
    .bw = SX126X_LORA_BW_125,
    .cr = SX126X_LORA_CR_4_5,
    .ldro = 1
};

static sx126x_pkt_params_lora_t lora_pkt_params = {
    .preamble_len_in_symb = 8,
    .header_type = SX126X_LORA_PKT_EXPLICIT,
    .pld_len_in_bytes = 64,
    .crc_is_on = true,
    .invert_iq_is_on = false
};

// Debug function
static void lora_debug_print(const char* message) {
    HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), 1000);
    HAL_UART_Transmit(&huart4, (uint8_t*)message, strlen(message), 1000);
}

// Detect LoRa module presence using real SX126x commands
static int8_t lora_detect_module(void) {
    sx126x_chip_status_t chip_status;
    
    lora_debug_print("Detecting LoRa module...\r\n");
    
    // Reset module first
    sx126x_reset(NULL);
    
    // Try to get chip status - this will fail if no module is present
    sx126x_status_t status = sx126x_get_status(NULL, &chip_status);
    
    if (status == SX126X_STATUS_OK) {
        lora_debug_print("✓ LoRa module detected on SPI bus\r\n");
        lora_debug_print("  Chip Mode: ");
        switch (chip_status.chip_mode) {
            case SX126X_CHIP_MODE_STBY_RC:
                lora_debug_print("Standby RC\r\n");
                break;
            case SX126X_CHIP_MODE_STBY_XOSC:
                lora_debug_print("Standby XOSC\r\n");
                break;
            case SX126X_CHIP_MODE_FS:
                lora_debug_print("Frequency Synthesis\r\n");
                break;
            case SX126X_CHIP_MODE_RX:
                lora_debug_print("Receive\r\n");
                break;
            case SX126X_CHIP_MODE_TX:
                lora_debug_print("Transmit\r\n");
                break;
            default:
                lora_debug_print("Unknown\r\n");
                break;
        }
        lora_module_detected = 1;
        return 0;
    } else {
        lora_debug_print("✗ No LoRa module detected on SPI bus\r\n");
        lora_debug_print("Troubleshooting steps:\r\n");
        lora_debug_print("  1. Check SPI connections:\r\n");
        lora_debug_print("     - PA5 (SCK) → LoRa SCK\r\n");
        lora_debug_print("     - PA6 (MISO) → LoRa MISO\r\n");
        lora_debug_print("     - PA7 (MOSI) → LoRa MOSI\r\n");
        lora_debug_print("     - PA4 (NSS) → LoRa NSS\r\n");
        lora_debug_print("     - PC0 (RESET) → LoRa RESET\r\n");
        lora_debug_print("  2. Verify power supply:\r\n");
        lora_debug_print("     - LoRa VCC → 3.3V\r\n");
        lora_debug_print("     - LoRa GND → GND\r\n");
        lora_debug_print("  3. Check module type (SX1261/SX1262/SX1268)\r\n");
        lora_module_detected = 0;
        return -1;
    }
}

// Initialize LoRa module using real SX126x driver
int8_t lora_init(void) {
    sx126x_status_t status;
    
    lora_debug_print("Initializing LoRa module...\r\n");
    
    // First detect if module is present
    if (lora_detect_module() != 0) {
        lora_debug_print("✗ LoRa initialization failed - no module detected\r\n");
        lora_initialized = 0;
        return -1;
    }
    
    // Set standby mode
    status = sx126x_set_standby(NULL, SX126X_STANDBY_CFG_RC);
    if (status != SX126X_STATUS_OK) {
        lora_debug_print("✗ Failed to set standby mode\r\n");
        lora_initialized = 0;
        return -1;
    }
    
    // Set regulator mode to LDO
    status = sx126x_set_reg_mode(NULL, SX126X_REG_MODE_LDO);
    if (status != SX126X_STATUS_OK) {
        lora_debug_print("✗ Failed to set regulator mode\r\n");
        lora_initialized = 0;
        return -1;
    }
    
    // Set packet type to LoRa
    status = sx126x_set_pkt_type(NULL, SX126X_PKT_TYPE_LORA);
    if (status != SX126X_STATUS_OK) {
        lora_debug_print("✗ Failed to set packet type\r\n");
        lora_initialized = 0;
        return -1;
    }
    
    // Set RF frequency (868 MHz)
    status = sx126x_set_rf_freq(NULL, LORA_FREQUENCY_HZ);
    if (status != SX126X_STATUS_OK) {
        lora_debug_print("✗ Failed to set RF frequency\r\n");
        lora_initialized = 0;
        return -1;
    }
    
    // Set LoRa modulation parameters
    status = sx126x_set_lora_mod_params(NULL, &lora_mod_params);
    if (status != SX126X_STATUS_OK) {
        lora_debug_print("✗ Failed to set LoRa modulation parameters\r\n");
        lora_initialized = 0;
        return -1;
    }
    
    // Set LoRa packet parameters
    status = sx126x_set_lora_pkt_params(NULL, &lora_pkt_params);
    if (status != SX126X_STATUS_OK) {
        lora_debug_print("✗ Failed to set LoRa packet parameters\r\n");
        lora_initialized = 0;
        return -1;
    }
    
    // Set TX parameters
    status = sx126x_set_tx_params(NULL, LORA_TX_POWER_DBM, SX126X_RAMP_10_US);
    if (status != SX126X_STATUS_OK) {
        lora_debug_print("✗ Failed to set TX parameters\r\n");
        lora_initialized = 0;
        return -1;
    }
    
    // Set LoRa sync word
    status = sx126x_set_lora_sync_word(NULL, LORA_SYNC_WORD);
    if (status != SX126X_STATUS_OK) {
        lora_debug_print("✗ Failed to set LoRa sync word\r\n");
        lora_initialized = 0;
        return -1;
    }
    
    // Set buffer base address
    status = sx126x_set_buffer_base_address(NULL, 0x00, 0x00);
    if (status != SX126X_STATUS_OK) {
        lora_debug_print("✗ Failed to set buffer base address\r\n");
        lora_initialized = 0;
        return -1;
    }
    
    // Set DIO IRQ parameters
    status = sx126x_set_dio_irq_params(NULL, SX126X_IRQ_TX_DONE | SX126X_IRQ_TIMEOUT, 
                                       SX126X_IRQ_TX_DONE | SX126X_IRQ_TIMEOUT, 0, 0);
    if (status != SX126X_STATUS_OK) {
        lora_debug_print("✗ Failed to set DIO IRQ parameters\r\n");
        lora_initialized = 0;
        return -1;
    }
    
    lora_debug_print("✓ LoRa module initialized successfully\r\n");
    lora_debug_print("  Frequency: 868 MHz\r\n");
    lora_debug_print("  Spreading Factor: SF7\r\n");
    lora_debug_print("  Bandwidth: 125 kHz\r\n");
    lora_debug_print("  Coding Rate: 4/5\r\n");
    lora_debug_print("  TX Power: 14 dBm\r\n");
    lora_initialized = 1;
    return 0;
}

// Send sensor data via LoRa
int8_t lora_send_sensor_data(float temperature, float pressure, float humidity) {
    if (!lora_module_detected) {
        lora_debug_print("✗ LoRa transmission failed - no module detected\r\n");
        return -1;
    }
    
    if (!lora_initialized) {
        lora_debug_print("✗ LoRa transmission failed - module not initialized\r\n");
        return -1;
    }
    
    char payload[LORA_PAYLOAD_LENGTH];
    int len;
    
    // Format sensor data as JSON-like string
    len = snprintf(payload, sizeof(payload), 
                   "{\"temp\":%.2f,\"press\":%.2f,\"hum\":%.2f,\"node\":\"STM32\"}",
                   temperature, pressure, humidity);
    
    if (len < 0 || len >= sizeof(payload)) {
        lora_debug_print("Error: Payload too long\r\n");
        return -1;
    }
    
    return lora_send_message((uint8_t*)payload, len);
}

// Send message via LoRa using real SX126x driver
int8_t lora_send_message(const uint8_t* data, uint8_t length) {
    sx126x_status_t status;
    sx126x_irq_mask_t irq_status;
    char debug_msg[128];
    
    if (!lora_module_detected) {
        lora_debug_print("✗ LoRa transmission failed - no module detected\r\n");
        return -1;
    }
    
    if (!lora_initialized) {
        lora_debug_print("✗ LoRa transmission failed - module not initialized\r\n");
        return -1;
    }
    
    if (data == NULL || length == 0 || length > LORA_PAYLOAD_LENGTH) {
        lora_debug_print("Invalid LoRa message parameters\r\n");
        return -1;
    }
    
    lora_debug_print("Sending LoRa message...\r\n");
    
    // Update packet length
    lora_pkt_params.pld_len_in_bytes = length;
    status = sx126x_set_lora_pkt_params(NULL, &lora_pkt_params);
    if (status != SX126X_STATUS_OK) {
        lora_debug_print("✗ Failed to update packet parameters\r\n");
        return -1;
    }
    
    // Write payload to buffer
    status = sx126x_write_buffer(NULL, 0x00, data, length);
    if (status != SX126X_STATUS_OK) {
        lora_debug_print("✗ Failed to write payload to buffer\r\n");
        return -1;
    }
    
    // Clear IRQ status
    status = sx126x_clear_irq_status(NULL, SX126X_IRQ_ALL);
    if (status != SX126X_STATUS_OK) {
        lora_debug_print("✗ Failed to clear IRQ status\r\n");
        return -1;
    }
    
    // Start transmission
    status = sx126x_set_tx(NULL, 1000); // 1 second timeout
    if (status != SX126X_STATUS_OK) {
        lora_debug_print("✗ Failed to start transmission\r\n");
        return -1;
    }
    
    // Wait for transmission to complete
    uint32_t timeout = HAL_GetTick() + 2000; // 2 second timeout
    while (HAL_GetTick() < timeout) {
        status = sx126x_get_irq_status(NULL, &irq_status);
        if (status == SX126X_STATUS_OK) {
            if (irq_status & SX126X_IRQ_TX_DONE) {
                lora_debug_print("✓ LoRa transmission completed successfully\r\n");
                snprintf(debug_msg, sizeof(debug_msg), "Message sent: %.*s\r\n", length, data);
                lora_debug_print(debug_msg);
                return 0;
            } else if (irq_status & SX126X_IRQ_TIMEOUT) {
                lora_debug_print("✗ LoRa transmission timeout\r\n");
                return -1;
            }
        }
        HAL_Delay(1);
    }
    
    lora_debug_print("✗ LoRa transmission timeout\r\n");
    return -1;
}

// Process LoRa interrupts
void lora_process_irq(void) {
    if (!lora_module_detected) {
        return; // No module to process interrupts for
    }
    
    sx126x_irq_mask_t irq_status;
    sx126x_status_t status = sx126x_get_and_clear_irq_status(NULL, &irq_status);
    
    if (status == SX126X_STATUS_OK && irq_status != 0) {
        if (irq_status & SX126X_IRQ_TX_DONE) {
            lora_debug_print("IRQ: Transmission completed\r\n");
        }
        if (irq_status & SX126X_IRQ_TIMEOUT) {
            lora_debug_print("IRQ: Transmission timeout\r\n");
        }
    }
}

// Get LoRa status
int8_t lora_get_status(void) {
    if (!lora_module_detected) {
        lora_debug_print("LoRa Status: No module detected\r\n");
        return -1;
    }
    
    if (!lora_initialized) {
        lora_debug_print("LoRa Status: Module detected but not initialized\r\n");
        return -2;
    }
    
    sx126x_chip_status_t chip_status;
    sx126x_status_t status = sx126x_get_status(NULL, &chip_status);
    
    if (status == SX126X_STATUS_OK) {
        lora_debug_print("LoRa Status: Module ready\r\n");
        lora_debug_print("  Chip Mode: ");
        switch (chip_status.chip_mode) {
            case SX126X_CHIP_MODE_STBY_RC:
                lora_debug_print("Standby RC\r\n");
                break;
            case SX126X_CHIP_MODE_STBY_XOSC:
                lora_debug_print("Standby XOSC\r\n");
                break;
            case SX126X_CHIP_MODE_FS:
                lora_debug_print("Frequency Synthesis\r\n");
                break;
            case SX126X_CHIP_MODE_RX:
                lora_debug_print("Receive\r\n");
                break;
            case SX126X_CHIP_MODE_TX:
                lora_debug_print("Transmit\r\n");
                break;
            default:
                lora_debug_print("Unknown\r\n");
                break;
        }
        return 0;
    } else {
        lora_debug_print("LoRa Status: Error reading chip status\r\n");
        return -3;
    }
}

// Print LoRa configuration
void lora_print_config(void) {
    lora_debug_print("=== LoRa Configuration ===\r\n");
    
    if (!lora_module_detected) {
        lora_debug_print("Status: No module detected\r\n");
        lora_debug_print("========================\r\n");
        return;
    }
    
    lora_debug_print("Status: Module detected\r\n");
    lora_debug_print("Initialized: ");
    lora_debug_print(lora_initialized ? "Yes\r\n" : "No\r\n");
    lora_debug_print("Frequency: 868 MHz (EU band)\r\n");
    lora_debug_print("Spreading Factor: SF7\r\n");
    lora_debug_print("Bandwidth: 125 kHz\r\n");
    lora_debug_print("Coding Rate: 4/5\r\n");
    lora_debug_print("TX Power: 14 dBm\r\n");
    lora_debug_print("Sync Word: 0x12\r\n");
    lora_debug_print("Payload Length: 64 bytes\r\n");
    lora_debug_print("Preamble Length: 8 symbols\r\n");
    lora_debug_print("CRC: Enabled\r\n");
    lora_debug_print("IQ Inversion: Disabled\r\n");
    lora_debug_print("========================\r\n");
}

// Test LoRa functionality
int8_t lora_test_transmission(void) {
    lora_debug_print("=== LoRa Transmission Test ===\r\n");
    
    if (!lora_module_detected) {
        lora_debug_print("✗ Test failed - no LoRa module detected\r\n");
        lora_debug_print("==============================\r\n");
        return -1;
    }
    
    if (!lora_initialized) {
        lora_debug_print("✗ Test failed - LoRa module not initialized\r\n");
        lora_debug_print("==============================\r\n");
        return -2;
    }
    
    // Test with a simple message
    const char* test_msg = "LoRa Test Message from STM32";
    int8_t result = lora_send_message((uint8_t*)test_msg, strlen(test_msg));
    
    if (result == 0) {
        lora_debug_print("✓ LoRa test transmission successful\r\n");
    } else {
        lora_debug_print("✗ LoRa test transmission failed\r\n");
    }
    
    lora_debug_print("==============================\r\n");
    return result;
} 