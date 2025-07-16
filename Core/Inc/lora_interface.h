#ifndef __LORA_INTERFACE_H__
#define __LORA_INTERFACE_H__

#include "stm32g0xx_hal.h"

// SX1262 LoRa configuration
#define SX1262_FREQUENCY_HZ        868000000  // 868 MHz (EU band)
#define SX1262_TX_POWER_DBM        14         // 14 dBm output power
#define SX1262_SPREADING_FACTOR    7          // SF7
#define SX1262_BANDWIDTH           125        // 125 kHz
#define SX1262_CODING_RATE         1          // 4/5
#define SX1262_PREAMBLE_LENGTH     8
#define SX1262_PAYLOAD_LENGTH      64
#define SX1262_SYNC_WORD           0x12

// SX1262 context structure for SPI communication
typedef struct {
    SPI_HandleTypeDef* spi;
    GPIO_TypeDef* nss_port;
    uint16_t nss_pin;
    GPIO_TypeDef* reset_port;
    uint16_t reset_pin;
    GPIO_TypeDef* busy_port;
    uint16_t busy_pin;
} sx1262_context_t;

// Function prototypes
int8_t sx1262_init(void);
int8_t sx1262_send_sensor_data(float temperature, float pressure, float humidity);
int8_t sx1262_send_message(const uint8_t* data, uint8_t length);
void sx1262_process_irq(void);
int8_t sx1262_get_status(void);
void sx1262_print_config(void);
int8_t sx1262_test_transmission(void);

// SX1262 SPI functions
int8_t sx1262_spi_read(uint8_t address, uint8_t* data, uint8_t length);
int8_t sx1262_spi_write(uint8_t address, const uint8_t* data, uint8_t length);
int8_t sx1262_spi_write_read(uint8_t address, const uint8_t* tx_data, uint8_t* rx_data, uint8_t length);
uint8_t sx1262_wait_for_busy(void);
int8_t sx1262_reset(void);
int8_t sx1262_configure_lora(void);
int8_t sx1262_detect_module(void);
int8_t sx1262_test_spi(void);

// Legacy function names for compatibility
#define lora_init sx1262_init
#define lora_send_sensor_data sx1262_send_sensor_data
#define lora_send_message sx1262_send_message
#define lora_process_irq sx1262_process_irq
#define lora_get_status sx1262_get_status
#define lora_print_config sx1262_print_config
#define lora_test_transmission sx1262_test_transmission

// Additional legacy functions for compatibility
int8_t lora_scan_signals(uint32_t scan_time_ms);
int8_t lora_start_monitoring(void);
int8_t lora_stop_monitoring(void);
int8_t lora_get_rssi(void);

#endif // __LORA_INTERFACE_H__ 