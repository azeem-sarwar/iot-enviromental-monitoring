#ifndef __LORA_INTERFACE_H__
#define __LORA_INTERFACE_H__

#include "stm32g0xx_hal.h"

// LoRa configuration
#define LORA_FREQUENCY_HZ        868000000  // 868 MHz (EU band)
#define LORA_TX_POWER_DBM        14         // 14 dBm output power
#define LORA_SPREADING_FACTOR    7          // SF7
#define LORA_BANDWIDTH           125        // 125 kHz
#define LORA_CODING_RATE         1          // 4/5
#define LORA_PREAMBLE_LENGTH     8
#define LORA_PAYLOAD_LENGTH      64
#define LORA_SYNC_WORD           0x12

// LoRa context structure
typedef struct {
    SPI_HandleTypeDef* spi;
    GPIO_TypeDef* nss_port;
    uint16_t nss_pin;
    GPIO_TypeDef* reset_port;
    uint16_t reset_pin;
} lora_context_t;

// Function prototypes
int8_t lora_init(void);
int8_t lora_send_sensor_data(float temperature, float pressure, float humidity);
int8_t lora_send_message(const uint8_t* data, uint8_t length);
void lora_process_irq(void);
int8_t lora_get_status(void);
int8_t lora_force_redetect(void);
void lora_print_config(void);
int8_t lora_test_transmission(void);

// New scanning and monitoring functions
int8_t lora_scan_signals(uint32_t scan_time_ms);
int8_t lora_start_monitoring(void);
int8_t lora_stop_monitoring(void);
int8_t lora_get_rssi(void);

#endif // __LORA_INTERFACE_H__ 