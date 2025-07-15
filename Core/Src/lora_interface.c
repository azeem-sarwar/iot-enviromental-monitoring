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
    
    // Reset module first
    sx126x_reset(NULL);
    HAL_Delay(50); // Give reset time to take effect
    
    // Test 1: Try to get chip status
    sx126x_status_t status = sx126x_get_status(NULL, &chip_status);
    if (status != SX126X_STATUS_OK) {
        lora_debug_print("✗ LoRa detection failed: get_status returned error\r\n");
        lora_module_detected = 0;
        return -1;
    }
    
    // Test 2: Try to set standby mode
    status = sx126x_set_standby(NULL, SX126X_STANDBY_CFG_RC);
    if (status != SX126X_STATUS_OK) {
        lora_debug_print("✗ LoRa detection failed: set_standby returned error\r\n");
        lora_module_detected = 0;
        return -1;
    }
    
    // Test 3: Try to read a register (this is the most reliable test)
    uint8_t test_buffer[1];
    status = sx126x_read_register(NULL, 0x0000, test_buffer, 1);
    if (status != SX126X_STATUS_OK) {
        lora_debug_print("✗ LoRa detection failed: read_register returned error\r\n");
        lora_module_detected = 0;
        return -1;
    }
    
    // Test 4: Try to set packet type (this requires actual chip response)
    status = sx126x_set_pkt_type(NULL, SX126X_PKT_TYPE_LORA);
    if (status != SX126X_STATUS_OK) {
        lora_debug_print("✗ LoRa detection failed: set_pkt_type returned error\r\n");
        lora_module_detected = 0;
        return -1;
    }
    
    // Test 5: Try to get packet type back (verifies the chip actually processed the command)
    sx126x_pkt_type_t pkt_type;
    status = sx126x_get_pkt_type(NULL, &pkt_type);
    if (status != SX126X_STATUS_OK || pkt_type != SX126X_PKT_TYPE_LORA) {
        lora_debug_print("✗ LoRa detection failed: get_pkt_type verification failed\r\n");
        lora_module_detected = 0;
        return -1;
    }
    
    // All tests passed - module is definitely present
    lora_module_detected = 1;
    lora_debug_print("✓ LoRa module detected successfully\r\n");
    return 0;
}

// Initialize LoRa module using real SX126x driver
int8_t lora_init(void) {
    sx126x_status_t status;
    
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
        return 0;
    } else {
        lora_debug_print("LoRa Status: Error reading chip status\r\n");
        return -3;
    }
}

// Force re-detection of LoRa module
int8_t lora_force_redetect(void) {
    lora_debug_print("Forcing LoRa module re-detection...\r\n");
    
    // Reset detection flags
    lora_module_detected = 0;
    lora_initialized = 0;
    
    // Try to detect module
    int8_t result = lora_detect_module();
    
    if (result == 0) {
        lora_debug_print("✓ LoRa module detected and verified\r\n");
        
        // Additional verification: try to initialize
        lora_debug_print("Testing full initialization...\r\n");
        if (lora_init() == 0) {
            lora_debug_print("✓ LoRa module fully functional\r\n");
        } else {
            lora_debug_print("✗ LoRa module detected but initialization failed\r\n");
            lora_initialized = 0;
        }
    } else {
        lora_debug_print("✗ LoRa module not detected - check wiring:\r\n");
        lora_debug_print("  - PA5 (SCK) → LoRa SCK\r\n");
        lora_debug_print("  - PA6 (MISO) → LoRa MISO\r\n");
        lora_debug_print("  - PA7 (MOSI) → LoRa MOSI\r\n");
        lora_debug_print("  - PA4 (NSS) → LoRa NSS\r\n");
        lora_debug_print("  - PC0 (RESET) → LoRa RESET\r\n");
        lora_debug_print("  - 3.3V → LoRa VCC\r\n");
        lora_debug_print("  - GND → LoRa GND\r\n");
    }
    
    return result;
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
    if (!lora_module_detected) {
        lora_debug_print("✗ Test failed - no LoRa module detected\r\n");
        return -1;
    }
    
    if (!lora_initialized) {
        lora_debug_print("✗ Test failed - LoRa module not initialized\r\n");
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
    
    return result;
}

// Scan for LoRa signals from other devices
int8_t lora_scan_signals(uint32_t scan_time_ms) {
    sx126x_status_t status;
    sx126x_irq_mask_t irq_status;
    uint8_t rx_buffer[256];
    uint8_t payload_length;
    sx126x_pkt_status_lora_t pkt_status;
    char scan_msg[128];
    
    if (!lora_module_detected) {
        lora_debug_print("✗ Scan failed - no LoRa module detected\r\n");
        return -1;
    }
    
    if (!lora_initialized) {
        lora_debug_print("✗ Scan failed - LoRa module not initialized\r\n");
        return -2;
    }
    
    // Set to receive mode
    status = sx126x_set_rx(NULL, scan_time_ms);
    if (status != SX126X_STATUS_OK) {
        lora_debug_print("✗ Failed to set receive mode\r\n");
        return -3;
    }
    
    snprintf(scan_msg, sizeof(scan_msg), "Scanning for LoRa signals for %lu ms...\r\n", scan_time_ms);
    lora_debug_print(scan_msg);
    
    // Wait for reception or timeout
    uint32_t start_time = HAL_GetTick();
    uint32_t timeout = start_time + scan_time_ms;
    
    while (HAL_GetTick() < timeout) {
        status = sx126x_get_irq_status(NULL, &irq_status);
        if (status == SX126X_STATUS_OK) {
            if (irq_status & SX126X_IRQ_RX_DONE) {
                // Packet received
                status = sx126x_get_lora_pkt_status(NULL, &pkt_status);
                if (status == SX126X_STATUS_OK) {
                    // Read payload
                    status = sx126x_read_buffer(NULL, 0x00, rx_buffer, payload_length);
                    if (status == SX126X_STATUS_OK) {
                        snprintf(scan_msg, sizeof(scan_msg), 
                                "✓ Signal detected! RSSI: %d dBm, SNR: %d dB, Length: %d bytes\r\n",
                                pkt_status.rssi_pkt_in_dbm, pkt_status.snr_pkt_in_db, payload_length);
                        lora_debug_print(scan_msg);
                        
                        // Print first 32 bytes of payload as hex
                        lora_debug_print("Payload (hex): ");
                        for (int i = 0; i < (payload_length > 32 ? 32 : payload_length); i++) {
                            snprintf(scan_msg, sizeof(scan_msg), "%02X ", rx_buffer[i]);
                            lora_debug_print(scan_msg);
                        }
                        lora_debug_print("\r\n");
                        
                        // Try to print as string if it looks like text
                        if (payload_length > 0) {
                            lora_debug_print("Payload (text): ");
                            for (int i = 0; i < (payload_length > 32 ? 32 : payload_length); i++) {
                                if (rx_buffer[i] >= 32 && rx_buffer[i] <= 126) {
                                    snprintf(scan_msg, sizeof(scan_msg), "%c", rx_buffer[i]);
                                    lora_debug_print(scan_msg);
                                } else {
                                    lora_debug_print(".");
                                }
                            }
                            lora_debug_print("\r\n");
                        }
                    }
                }
                
                // Clear IRQ and continue scanning
                sx126x_clear_irq_status(NULL, SX126X_IRQ_ALL);
                status = sx126x_set_rx(NULL, timeout - HAL_GetTick());
            } else if (irq_status & SX126X_IRQ_TIMEOUT) {
                break; // Scan timeout
            }
        }
        HAL_Delay(10);
    }
    
    lora_debug_print("Scan completed\r\n");
    return 0;
}

// Continuous LoRa monitoring mode
int8_t lora_start_monitoring(void) {
    sx126x_status_t status;
    
    if (!lora_module_detected) {
        lora_debug_print("✗ Monitoring failed - no LoRa module detected\r\n");
        return -1;
    }
    
    if (!lora_initialized) {
        lora_debug_print("✗ Monitoring failed - LoRa module not initialized\r\n");
        return -2;
    }
    
    lora_debug_print("Starting continuous LoRa monitoring...\r\n");
    lora_debug_print("Press any key to stop monitoring\r\n");
    
    // Set to continuous receive mode
    status = sx126x_set_rx(NULL, 0); // 0 = continuous RX
    if (status != SX126X_STATUS_OK) {
        lora_debug_print("✗ Failed to set continuous receive mode\r\n");
        return -3;
    }
    
    return 0;
}

// Stop LoRa monitoring
int8_t lora_stop_monitoring(void) {
    sx126x_status_t status;
    
    if (!lora_module_detected) {
        return -1;
    }
    
    // Set to standby mode
    status = sx126x_set_standby(NULL, SX126X_STANDBY_CFG_RC);
    if (status != SX126X_STATUS_OK) {
        lora_debug_print("✗ Failed to stop monitoring\r\n");
        return -1;
    }
    
    lora_debug_print("Monitoring stopped\r\n");
    return 0;
}

// Get RSSI of current channel (for signal strength measurement)
int8_t lora_get_rssi(void) {
    sx126x_status_t status;
    int16_t rssi;
    
    if (!lora_module_detected) {
        lora_debug_print("✗ RSSI measurement failed - no LoRa module detected\r\n");
        return -1;
    }
    
    if (!lora_initialized) {
        lora_debug_print("✗ RSSI measurement failed - LoRa module not initialized\r\n");
        return -2;
    }
    
    // Get RSSI of current channel
    status = sx126x_get_rssi_inst(NULL, &rssi);
    if (status == SX126X_STATUS_OK) {
        char rssi_msg[64];
        snprintf(rssi_msg, sizeof(rssi_msg), "Current RSSI: %d dBm\r\n", rssi);
        lora_debug_print(rssi_msg);
        return 0;
    } else {
        lora_debug_print("✗ Failed to get RSSI\r\n");
        return -3;
    }
} 