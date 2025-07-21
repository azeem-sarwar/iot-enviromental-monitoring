#ifndef __BME680_INTERFACE_H__
#define __BME680_INTERFACE_H__

#include "bme680/bme68x.h"
#include "stm32g0xx_hal.h"

// BME680 device structure
extern struct bme68x_dev bme680_dev;

// Function prototypes
int8_t bme680_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
int8_t bme680_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);
void bme680_delay_us(uint32_t period, void *intf_ptr);
int8_t bme680_init_sensor(void);
int8_t bme680_read_sensor_data(struct bme68x_data *data);
void bme680_print_sensor_data(struct bme68x_data *data);
void bme680_test_sensor(void);
int8_t bme680_check_sensor_presence(void);
void i2c_scan_bus(void);
void bme680_read_raw_registers(void);
void bme680_read_raw_adc_values(void);
void bme680_check_calibration_data(void);
float decode_ieee754(uint32_t hex_value);
void bme680_comprehensive_diagnostic(void);

#endif // __BME680_INTERFACE_H__ 