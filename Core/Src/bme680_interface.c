#include "bme680_interface.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart4;

// BME680 device structure
struct bme68x_dev bme680_dev;

// Debug function to send message to both UARTs
void debug_print(const char* message) {
    HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart4, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
}

// I2C bus scanner function
void i2c_scan_bus(void) {
    uint8_t found_devices = 0;
    char debug_msg[128];
    
    debug_print("Scanning I2C bus for devices...\r\n");
    
    for (uint8_t addr = 1; addr < 128; addr++) {
        HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(&hi2c1, addr << 1, 2, 100);
        if (status == HAL_OK) {
            snprintf(debug_msg, sizeof(debug_msg), "Device found at address: 0x%02X\r\n", addr);
            debug_print(debug_msg);
            found_devices++;
        }
    }
    
    if (found_devices == 0) {
        debug_print("No I2C devices found on bus!\r\n");
    } else {
        snprintf(debug_msg, sizeof(debug_msg), "Total devices found: %d\r\n", found_devices);
        debug_print(debug_msg);
    }
}

// Enhanced I2C read function for BME680 with detailed error reporting
int8_t bme680_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    HAL_StatusTypeDef status;
    char debug_msg[128];
    
    // Debug: Print read attempt
    snprintf(debug_msg, sizeof(debug_msg), "I2C Read: Reg=0x%02X, Len=%lu\r\n", reg_addr, len);
    debug_print(debug_msg);
    
    // Read data from BME680 using I2C
    status = HAL_I2C_Mem_Read(&hi2c1, BME68X_I2C_ADDR_LOW << 1, reg_addr, 
                              I2C_MEMADD_SIZE_8BIT, reg_data, len, 1000);
    
    if (status == HAL_OK) {
        snprintf(debug_msg, sizeof(debug_msg), "I2C Read Success: Data[0]=0x%02X\r\n", reg_data[0]);
        debug_print(debug_msg);
        return BME68X_OK;
    } else {
        snprintf(debug_msg, sizeof(debug_msg), "I2C Read Failed: Status=%d\r\n", status);
        debug_print(debug_msg);
        return BME68X_E_COM_FAIL;
    }
}

// Enhanced I2C write function for BME680 with detailed error reporting
int8_t bme680_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    HAL_StatusTypeDef status;
    char debug_msg[128];
    
    // Debug: Print write attempt
    snprintf(debug_msg, sizeof(debug_msg), "I2C Write: Reg=0x%02X, Data[0]=0x%02X, Len=%lu\r\n", 
             reg_addr, reg_data[0], len);
    debug_print(debug_msg);
    
    // Write data to BME680 using I2C
    status = HAL_I2C_Mem_Write(&hi2c1, BME68X_I2C_ADDR_LOW << 1, reg_addr, 
                               I2C_MEMADD_SIZE_8BIT, (uint8_t*)reg_data, len, 1000);
    
    if (status == HAL_OK) {
        debug_print("I2C Write Success\r\n");
        return BME68X_OK;
    } else {
        snprintf(debug_msg, sizeof(debug_msg), "I2C Write Failed: Status=%d\r\n", status);
        debug_print(debug_msg);
        return BME68X_E_COM_FAIL;
    }
}

// Delay function for BME680
void bme680_delay_us(uint32_t period, void *intf_ptr)
{
    // For small delays, use a simple loop
    // For larger delays, use HAL_Delay
    if (period < 1000) {
        // Simple microsecond delay loop
        volatile uint32_t i;
        for (i = 0; i < period * 16; i++) {
            __NOP();
        }
    } else {
        // Convert microseconds to milliseconds for HAL_Delay
        HAL_Delay(period / 1000);
    }
}

// Enhanced sensor presence check with multiple address attempts
int8_t bme680_check_sensor_presence(void)
{
    HAL_StatusTypeDef status;
    uint8_t chip_id;
    char debug_msg[128];
    
    debug_print("Checking BME680 sensor presence...\r\n");
    
    // Try both possible addresses
    uint8_t addresses[] = {BME68X_I2C_ADDR_LOW, BME68X_I2C_ADDR_HIGH};
    const char* addr_names[] = {"0x76", "0x77"};
    
    for (int i = 0; i < 2; i++) {
        snprintf(debug_msg, sizeof(debug_msg), "Trying address %s...\r\n", addr_names[i]);
        debug_print(debug_msg);
        
        // First check if device responds
        status = HAL_I2C_IsDeviceReady(&hi2c1, addresses[i] << 1, 3, 1000);
        if (status == HAL_OK) {
            snprintf(debug_msg, sizeof(debug_msg), "Device responds at address %s\r\n", addr_names[i]);
            debug_print(debug_msg);
            
            // Try to read chip ID
            status = HAL_I2C_Mem_Read(&hi2c1, addresses[i] << 1, BME68X_REG_CHIP_ID, 
                                      I2C_MEMADD_SIZE_8BIT, &chip_id, 1, 1000);
            
            if (status == HAL_OK) {
                snprintf(debug_msg, sizeof(debug_msg), "Chip ID read: 0x%02X (Expected: 0x%02X)\r\n", 
                         chip_id, BME68X_CHIP_ID);
                debug_print(debug_msg);
                
                if (chip_id == BME68X_CHIP_ID) {
                    snprintf(debug_msg, sizeof(debug_msg), "✓ BME680 found at address %s\r\n", addr_names[i]);
                    debug_print(debug_msg);
                    return BME68X_OK;
                } else {
                    snprintf(debug_msg, sizeof(debug_msg), "✗ Wrong chip ID at address %s\r\n", addr_names[i]);
                    debug_print(debug_msg);
                }
            } else {
                snprintf(debug_msg, sizeof(debug_msg), "✗ Failed to read chip ID at address %s\r\n", addr_names[i]);
                debug_print(debug_msg);
            }
        } else {
            snprintf(debug_msg, sizeof(debug_msg), "✗ No device at address %s\r\n", addr_names[i]);
            debug_print(debug_msg);
        }
    }
    
    debug_print("✗ BME680 sensor not found on any address\r\n");
    return BME68X_E_DEV_NOT_FOUND;
}

// Enhanced sensor initialization with detailed error reporting
int8_t bme680_init_sensor(void)
{
    int8_t rslt;
    char debug_msg[128];
    
    debug_print("Initializing BME680 sensor...\r\n");
    
    // Initialize device structure
    bme680_dev.intf = BME68X_I2C_INTF;
    bme680_dev.read = bme680_i2c_read;
    bme680_dev.write = bme680_i2c_write;
    bme680_dev.delay_us = bme680_delay_us;
    bme680_dev.intf_ptr = NULL;
    bme680_dev.amb_temp = 25;
    
    // Initialize the sensor
    rslt = bme68x_init(&bme680_dev);
    
    snprintf(debug_msg, sizeof(debug_msg), "BME68X Init Result: %d\r\n", rslt);
    debug_print(debug_msg);
    
    if (rslt == BME68X_OK) {
        debug_print("✓ BME68X library initialization successful\r\n");
        
        // Configure sensor settings
        struct bme68x_conf conf;
        conf.os_hum = BME68X_OS_1X;
        conf.os_pres = BME68X_OS_1X;
        conf.os_temp = BME68X_OS_1X;
        conf.filter = BME68X_FILTER_OFF;
        conf.odr = BME68X_ODR_NONE;
        
        rslt = bme68x_set_conf(&conf, &bme680_dev);
        
        snprintf(debug_msg, sizeof(debug_msg), "BME68X Config Result: %d\r\n", rslt);
        debug_print(debug_msg);
        
        if (rslt == BME68X_OK) {
            debug_print("✓ BME68X configuration successful\r\n");
            
            // Configure gas sensor (optional - for gas resistance measurement)
            struct bme68x_heatr_conf heatr_conf;
            heatr_conf.enable = BME68X_DISABLE; // Disable gas sensor for now
            heatr_conf.heatr_temp = 300;
            heatr_conf.heatr_dur = 100;
            
            rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme680_dev);
            
            snprintf(debug_msg, sizeof(debug_msg), "BME68X Gas Config Result: %d\r\n", rslt);
            debug_print(debug_msg);
            
            if (rslt == BME68X_OK) {
                debug_print("✓ BME680 sensor fully initialized\r\n");
            } else {
                debug_print("⚠ Gas sensor configuration failed, but sensor is usable\r\n");
            }
        } else {
            debug_print("✗ BME68X configuration failed\r\n");
        }
    } else {
        debug_print("✗ BME68X library initialization failed\r\n");
    }
    
    return rslt;
}

// Manual IEEE 754 decoder for debugging
float manual_decode_ieee754(uint32_t hex_value) {
    // Extract sign, exponent, and mantissa
    uint32_t sign = (hex_value >> 31) & 0x1;
    uint32_t exponent = (hex_value >> 23) & 0xFF;
    uint32_t mantissa = hex_value & 0x7FFFFF;
    
    // Handle special cases
    if (exponent == 0) {
        return 0.0f; // Zero
    }
    if (exponent == 0xFF) {
        if (mantissa == 0) {
            return sign ? -INFINITY : INFINITY; // Infinity
        } else {
            return NAN; // NaN
        }
    }
    
    // Normal case
    float result = 1.0f;
    for (int i = 22; i >= 0; i--) {
        if (mantissa & (1 << i)) {
            result += 1.0f / (1 << (23 - i));
        }
    }
    
    // Apply exponent
    int exp = (int)exponent - 127;
    if (exp > 0) {
        result *= (1 << exp);
    } else if (exp < 0) {
        result /= (1 << (-exp));
    }
    
    // Apply sign
    return sign ? -result : result;
}

// Decode IEEE 754 float value
float decode_ieee754(uint32_t hex_value) {
    union {
        uint32_t i;
        float f;
    } converter;
    converter.i = hex_value;
    return converter.f;
}

// Check if float is NaN or infinite
int is_float_invalid(float value) {
    union {
        float f;
        uint32_t i;
    } converter;
    converter.f = value;
    
    // Check for NaN (exponent all 1s, mantissa non-zero)
    if (((converter.i >> 23) & 0xFF) == 0xFF && (converter.i & 0x7FFFFF) != 0) {
        return 1; // NaN
    }
    
    // Check for infinite (exponent all 1s, mantissa zero)
    if (((converter.i >> 23) & 0xFF) == 0xFF && (converter.i & 0x7FFFFF) == 0) {
        return 2; // Infinite
    }
    
    return 0; // Valid
}

// Read sensor data
int8_t bme680_read_sensor_data(struct bme68x_data *data)
{
    int8_t rslt;
    uint8_t n_data;
    char debug_msg[128];
    
    debug_print("Reading BME680 sensor data...\r\n");
    
    // Set operation mode to forced mode
    rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &bme680_dev);
    
    if (rslt == BME68X_OK) {
        debug_print("✓ Forced mode set successfully\r\n");
        
        // Wait for measurement to complete
        uint32_t del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, NULL, &bme680_dev);
        snprintf(debug_msg, sizeof(debug_msg), "Measurement duration: %lu us\r\n", del_period);
        debug_print(debug_msg);
        
        bme680_dev.delay_us(del_period, bme680_dev.intf_ptr);
        
        // Read the data
        rslt = bme68x_get_data(BME68X_FORCED_MODE, data, &n_data, &bme680_dev);
        
        snprintf(debug_msg, sizeof(debug_msg), "Data read result: %d, samples: %d\r\n", rslt, n_data);
        debug_print(debug_msg);
        
        if (rslt == BME68X_OK && n_data > 0) {
            debug_print("✓ Sensor data read successfully\r\n");
            
            // Debug: Print raw values with more detail
            snprintf(debug_msg, sizeof(debug_msg), 
                     "Raw values - Temp: %.6f, Press: %.6f, Hum: %.6f\r\n",
                     data->temperature, data->pressure, data->humidity);
            debug_print(debug_msg);
            
            // Debug: Check if values are zero
            if (data->temperature == 0.0f && data->pressure == 0.0f && data->humidity == 0.0f) {
                debug_print("⚠ All sensor values are zero - possible library issue\r\n");
            }
            
            // Debug: Print memory representation and decoded values
            uint32_t temp_mem = *(uint32_t*)&data->temperature;
            uint32_t press_mem = *(uint32_t*)&data->pressure;
            uint32_t hum_mem = *(uint32_t*)&data->humidity;
            
            snprintf(debug_msg, sizeof(debug_msg), 
                     "Memory dump - Temp: %08lX, Press: %08lX, Hum: %08lX\r\n",
                     temp_mem, press_mem, hum_mem);
            debug_print(debug_msg);
            
            // Decode and show actual values using both methods
            float temp_decoded = decode_ieee754(temp_mem);
            float press_decoded = decode_ieee754(press_mem);
            float hum_decoded = decode_ieee754(hum_mem);
            
            // Manual decoding for comparison
            float temp_manual = manual_decode_ieee754(temp_mem);
            float press_manual = manual_decode_ieee754(press_mem);
            float hum_manual = manual_decode_ieee754(hum_mem);
            
            // Debug: Show hex values and decoded values directly
            snprintf(debug_msg, sizeof(debug_msg), 
                     "Hex values - Temp: 0x%08lX, Press: 0x%08lX, Hum: 0x%08lX\r\n",
                     temp_mem, press_mem, hum_mem);
            debug_print(debug_msg);
            
            snprintf(debug_msg, sizeof(debug_msg), 
                     "Decoded values (union) - Temp: %.6f, Press: %.6f, Hum: %.6f\r\n",
                     temp_decoded, press_decoded, hum_decoded);
            debug_print(debug_msg);
            
            snprintf(debug_msg, sizeof(debug_msg), 
                     "Decoded values (manual) - Temp: %.6f, Press: %.6f, Hum: %.6f\r\n",
                     temp_manual, press_manual, hum_manual);
            debug_print(debug_msg);
            
            // Debug: Show individual components
            snprintf(debug_msg, sizeof(debug_msg), 
                     "Temp breakdown - Hex: 0x%08lX, Decoded: %.6f\r\n",
                     temp_mem, temp_decoded);
            debug_print(debug_msg);
            
            // Direct IEEE 754 calculation for temperature
            uint32_t temp_sign = (temp_mem >> 31) & 0x1;
            uint32_t temp_exp = (temp_mem >> 23) & 0xFF;
            uint32_t temp_mant = temp_mem & 0x7FFFFF;
            
            snprintf(debug_msg, sizeof(debug_msg), 
                     "Temp IEEE 754 - Sign: %lu, Exp: %lu, Mant: 0x%06lX\r\n",
                     temp_sign, temp_exp, temp_mant);
            debug_print(debug_msg);
            
            if (temp_exp != 0 && temp_exp != 0xFF) {
                int temp_exp_val = (int)temp_exp - 127;
                float temp_result = 1.0f;
                
                // Calculate mantissa (implicit 1.0 + fractional part)
                for (int i = 22; i >= 0; i--) {
                    if (temp_mant & (1 << i)) {
                        temp_result += 1.0f / (1 << (23 - i));
                    }
                }
                
                // Apply exponent
                if (temp_exp_val > 0) {
                    temp_result *= (1 << temp_exp_val);
                } else if (temp_exp_val < 0) {
                    temp_result /= (1 << (-temp_exp_val));
                }
                
                // Apply sign
                if (temp_sign) temp_result = -temp_result;
                
                snprintf(debug_msg, sizeof(debug_msg), 
                         "Temp calculation - Exp_val: %d, Result: %.6f°C\r\n",
                         temp_exp_val, temp_result);
                debug_print(debug_msg);
                
                // Use this calculated value
                temp_decoded = temp_result;
            }
            
            // Also calculate pressure and humidity
            uint32_t press_sign = (press_mem >> 31) & 0x1;
            uint32_t press_exp = (press_mem >> 23) & 0xFF;
            uint32_t press_mant = press_mem & 0x7FFFFF;
            
            if (press_exp != 0 && press_exp != 0xFF) {
                int press_exp_val = (int)press_exp - 127;
                float press_result = 1.0f;
                
                for (int i = 22; i >= 0; i--) {
                    if (press_mant & (1 << i)) {
                        press_result += 1.0f / (1 << (23 - i));
                    }
                }
                
                if (press_exp_val > 0) {
                    press_result *= (1 << press_exp_val);
                } else if (press_exp_val < 0) {
                    press_result /= (1 << (-press_exp_val));
                }
                
                if (press_sign) press_result = -press_result;
                press_decoded = press_result;
            }
            
            uint32_t hum_sign = (hum_mem >> 31) & 0x1;
            uint32_t hum_exp = (hum_mem >> 23) & 0xFF;
            uint32_t hum_mant = hum_mem & 0x7FFFFF;
            
            if (hum_exp != 0 && hum_exp != 0xFF) {
                int hum_exp_val = (int)hum_exp - 127;
                float hum_result = 1.0f;
                
                for (int i = 22; i >= 0; i--) {
                    if (hum_mant & (1 << i)) {
                        hum_result += 1.0f / (1 << (23 - i));
                    }
                }
                
                if (hum_exp_val > 0) {
                    hum_result *= (1 << hum_exp_val);
                } else if (hum_exp_val < 0) {
                    hum_result /= (1 << (-hum_exp_val));
                }
                
                if (hum_sign) hum_result = -hum_result;
                hum_decoded = hum_result;
            }
            
            // Check validity of each value
            int temp_valid = is_float_invalid(data->temperature);
            int press_valid = is_float_invalid(data->pressure);
            int hum_valid = is_float_invalid(data->humidity);
            
            snprintf(debug_msg, sizeof(debug_msg), 
                     "Validity check - Temp: %s, Press: %s, Hum: %s\r\n",
                     temp_valid == 0 ? "Valid" : (temp_valid == 1 ? "NaN" : "Inf"),
                     press_valid == 0 ? "Valid" : (press_valid == 1 ? "NaN" : "Inf"),
                     hum_valid == 0 ? "Valid" : (hum_valid == 1 ? "NaN" : "Inf"));
            debug_print(debug_msg);
            
            // Check if values are valid (not NaN or infinite)
            if (is_float_invalid(data->temperature) || is_float_invalid(data->pressure) || is_float_invalid(data->humidity)) {
                debug_print("✗ Invalid sensor values detected (NaN or infinite)\r\n");
                return BME68X_E_INVALID_LENGTH;
            }
            
            // Check if values are within reasonable ranges
            if (data->temperature < -40.0f || data->temperature > 85.0f ||
                data->pressure < 30000.0f || data->pressure > 125000.0f ||
                data->humidity < 0.0f || data->humidity > 100.0f) {
                snprintf(debug_msg, sizeof(debug_msg), 
                         "⚠ Values out of expected range - Temp: %.2f, Press: %.2f, Hum: %.2f\r\n",
                         data->temperature, data->pressure, data->humidity);
                debug_print(debug_msg);
            }
            
            // Apply temperature offset correction if needed
            // The BME680 might have a factory offset that needs correction
            float temp_offset = -9.5f; // Adjust this value based on your testing
            data->temperature += temp_offset;
            
            snprintf(debug_msg, sizeof(debug_msg), 
                     "Temperature after offset correction: %.2f°C\r\n",
                     data->temperature);
            debug_print(debug_msg);
        } else {
            debug_print("✗ Failed to read sensor data\r\n");
        }
    } else {
        debug_print("✗ Failed to set forced mode\r\n");
    }
    
    return rslt;
}

// Print sensor data
void bme680_print_sensor_data(struct bme68x_data *data)
{
    char buffer[256];
    
    // Format and print temperature, pressure, and humidity
    snprintf(buffer, sizeof(buffer), 
             "Temperature: %.2f°C, Pressure: %.2f Pa, Humidity: %.2f%%\r\n",
             data->temperature, data->pressure, data->humidity);
    
    // Send via both UARTs
    HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart4, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}

// Check BME680 calibration data
void bme680_check_calibration_data(void)
{
    uint8_t calib_data[41];
    char debug_msg[128];
    
    debug_print("Reading BME680 calibration data...\r\n");
    
    // Read calibration data from registers 0xE1 to 0xF0 and 0x8A to 0xA1
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c1, BME68X_I2C_ADDR_LOW << 1, 0xE1, 
                                                I2C_MEMADD_SIZE_8BIT, calib_data, 16, 1000);
    
    if (status == HAL_OK) {
        status = HAL_I2C_Mem_Read(&hi2c1, BME68X_I2C_ADDR_LOW << 1, 0x8A, 
                                  I2C_MEMADD_SIZE_8BIT, &calib_data[16], 25, 1000);
    }
    
    if (status == HAL_OK) {
        debug_print("✓ Calibration data read successfully\r\n");
        
        // Display some key calibration values
        snprintf(debug_msg, sizeof(debug_msg), 
                 "T1: 0x%04X, T2: 0x%04X, T3: 0x%02X\r\n",
                 (calib_data[1] << 8) | calib_data[0],
                 (calib_data[3] << 8) | calib_data[2],
                 calib_data[3]);
        debug_print(debug_msg);
        
        snprintf(debug_msg, sizeof(debug_msg), 
                 "P1: 0x%04X, P2: 0x%04X, P3: 0x%02X\r\n",
                 (calib_data[5] << 8) | calib_data[4],
                 (calib_data[7] << 8) | calib_data[6],
                 calib_data[7]);
        debug_print(debug_msg);
        
        snprintf(debug_msg, sizeof(debug_msg), 
                 "H1: 0x%02X, H2: 0x%04X\r\n",
                 calib_data[25],
                 (calib_data[26] << 8) | calib_data[27]);
        debug_print(debug_msg);
    } else {
        debug_print("✗ Failed to read calibration data\r\n");
    }
}

// Read raw ADC values directly from BME680
void bme680_read_raw_adc_values(void)
{
    uint8_t raw_data[8];
    char debug_msg[128];
    
    debug_print("Reading raw BME680 ADC values...\r\n");
    
    // Read raw temperature, pressure, and humidity ADC values
    // Temperature: registers 0x22-0x24
    // Pressure: registers 0x1F-0x21  
    // Humidity: registers 0x25-0x26
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c1, BME68X_I2C_ADDR_LOW << 1, 0x1F, 
                                                I2C_MEMADD_SIZE_8BIT, raw_data, 8, 1000);
    
    if (status == HAL_OK) {
        // Extract raw ADC values
        uint32_t raw_temp = ((uint32_t)raw_data[3] << 12) | ((uint32_t)raw_data[4] << 4) | ((uint32_t)raw_data[5] >> 4);
        uint32_t raw_press = ((uint32_t)raw_data[0] << 12) | ((uint32_t)raw_data[1] << 4) | ((uint32_t)raw_data[2] >> 4);
        uint16_t raw_hum = ((uint16_t)raw_data[6] << 8) | raw_data[7];
        
        snprintf(debug_msg, sizeof(debug_msg), 
                 "Raw ADC values - Temp: %lu, Press: %lu, Hum: %u\r\n",
                 raw_temp, raw_press, raw_hum);
        debug_print(debug_msg);
        
        // Also read the calibration data to understand the conversion
        uint8_t calib_temp[3];
        status = HAL_I2C_Mem_Read(&hi2c1, BME68X_I2C_ADDR_LOW << 1, 0xE1, 
                                  I2C_MEMADD_SIZE_8BIT, calib_temp, 3, 1000);
        
        if (status == HAL_OK) {
            uint16_t T1 = (calib_temp[1] << 8) | calib_temp[0];
            int16_t T2 = (int16_t)((calib_temp[3] << 8) | calib_temp[2]);
            int8_t T3 = (int8_t)calib_temp[4];
            
            snprintf(debug_msg, sizeof(debug_msg), 
                     "Temp calibration - T1: %u, T2: %d, T3: %d\r\n",
                     T1, T2, T3);
            debug_print(debug_msg);
        }
    } else {
        debug_print("✗ Failed to read raw ADC values\r\n");
    }
}

// Simple direct register reading test
void bme680_read_raw_registers(void)
{
    uint8_t raw_data[8];
    char debug_msg[128];
    
    debug_print("Reading raw BME680 registers...\r\n");
    
    // Read temperature, pressure, and humidity registers (0x22-0x26)
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c1, BME68X_I2C_ADDR_LOW << 1, 0x22, 
                                                I2C_MEMADD_SIZE_8BIT, raw_data, 8, 1000);
    
    if (status == HAL_OK) {
        snprintf(debug_msg, sizeof(debug_msg), 
                 "Raw registers: 0x22=0x%02X, 0x23=0x%02X, 0x24=0x%02X, 0x25=0x%02X, 0x26=0x%02X\r\n",
                 raw_data[0], raw_data[1], raw_data[2], raw_data[3], raw_data[4]);
        debug_print(debug_msg);
        
        // Calculate raw temperature (24-bit value)
        uint32_t temp_raw = ((uint32_t)raw_data[0] << 12) | ((uint32_t)raw_data[1] << 4) | ((uint32_t)raw_data[2] >> 4);
        snprintf(debug_msg, sizeof(debug_msg), "Raw temperature value: 0x%06lX (%lu)\r\n", temp_raw, temp_raw);
        debug_print(debug_msg);
        
        // Calculate raw pressure (24-bit value)
        uint32_t press_raw = ((uint32_t)raw_data[3] << 12) | ((uint32_t)raw_data[4] << 4) | ((uint32_t)raw_data[5] >> 4);
        snprintf(debug_msg, sizeof(debug_msg), "Raw pressure value: 0x%06lX (%lu)\r\n", press_raw, press_raw);
        debug_print(debug_msg);
        
        // Calculate raw humidity (16-bit value)
        uint16_t hum_raw = ((uint16_t)raw_data[6] << 8) | raw_data[7];
        snprintf(debug_msg, sizeof(debug_msg), "Raw humidity value: 0x%04X (%u)\r\n", hum_raw, hum_raw);
        debug_print(debug_msg);
    } else {
        debug_print("Failed to read raw registers\r\n");
    }
}

// Test BME680 sensor
void bme680_test_sensor(void)
{
    struct bme68x_data sensor_data;
    char test_msg[256];
    
    debug_print("Testing BME680 sensor...\r\n");
    
    if (bme680_read_sensor_data(&sensor_data) == BME68X_OK) {
        // Decode all sensor values
        uint32_t temp_mem = *(uint32_t*)&sensor_data.temperature;
        uint32_t press_mem = *(uint32_t*)&sensor_data.pressure;
        uint32_t hum_mem = *(uint32_t*)&sensor_data.humidity;
        
        float temp_decoded = 0.0f, press_decoded = 0.0f, hum_decoded = 0.0f;
        
        // Decode temperature
        uint32_t temp_sign = (temp_mem >> 31) & 0x1;
        uint32_t temp_exp = (temp_mem >> 23) & 0xFF;
        uint32_t temp_mant = temp_mem & 0x7FFFFF;
        
        if (temp_exp != 0 && temp_exp != 0xFF) {
            int temp_exp_val = (int)temp_exp - 127;
            float temp_result = 1.0f;
            for (int i = 22; i >= 0; i--) {
                if (temp_mant & (1 << i)) {
                    temp_result += 1.0f / (1 << (23 - i));
                }
            }
            if (temp_exp_val > 0) temp_result *= (1 << temp_exp_val);
            else if (temp_exp_val < 0) temp_result /= (1 << (-temp_exp_val));
            if (temp_sign) temp_result = -temp_result;
            temp_decoded = temp_result;
        }
        
        // Decode pressure
        uint32_t press_sign = (press_mem >> 31) & 0x1;
        uint32_t press_exp = (press_mem >> 23) & 0xFF;
        uint32_t press_mant = press_mem & 0x7FFFFF;
        
        if (press_exp != 0 && press_exp != 0xFF) {
            int press_exp_val = (int)press_exp - 127;
            float press_result = 1.0f;
            for (int i = 22; i >= 0; i--) {
                if (press_mant & (1 << i)) {
                    press_result += 1.0f / (1 << (23 - i));
                }
            }
            if (press_exp_val > 0) press_result *= (1 << press_exp_val);
            else if (press_exp_val < 0) press_result /= (1 << (-press_exp_val));
            if (press_sign) press_result = -press_result;
            press_decoded = press_result;
        }
        
        // Decode humidity
        uint32_t hum_sign = (hum_mem >> 31) & 0x1;
        uint32_t hum_exp = (hum_mem >> 23) & 0xFF;
        uint32_t hum_mant = hum_mem & 0x7FFFFF;
        
        if (hum_exp != 0 && hum_exp != 0xFF) {
            int hum_exp_val = (int)hum_exp - 127;
            float hum_result = 1.0f;
            for (int i = 22; i >= 0; i--) {
                if (hum_mant & (1 << i)) {
                    hum_result += 1.0f / (1 << (23 - i));
                }
            }
            if (hum_exp_val > 0) hum_result *= (1 << hum_exp_val);
            else if (hum_exp_val < 0) hum_result /= (1 << (-hum_exp_val));
            if (hum_sign) hum_result = -hum_result;
            hum_decoded = hum_result;
        }
        
        snprintf(test_msg, sizeof(test_msg), 
                 "Test successful!\r\nTemperature: %.2f°C\r\nPressure: %.2f Pa\r\nHumidity: %.2f%%\r\n",
                 temp_decoded, press_decoded, hum_decoded);
    } else {
        snprintf(test_msg, sizeof(test_msg), "Test failed! Error reading sensor data.\r\n");
    }
    
    HAL_UART_Transmit(&huart2, (uint8_t*)test_msg, strlen(test_msg), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart4, (uint8_t*)test_msg, strlen(test_msg), HAL_MAX_DELAY);
} 

// Comprehensive BME680 diagnostic function
void bme680_comprehensive_diagnostic(void)
{
    char debug_msg[256];
    HAL_StatusTypeDef status;
    uint8_t chip_id;
    uint8_t raw_data[32];
    
    debug_print("\r\n=== BME680 COMPREHENSIVE DIAGNOSTIC ===\r\n");
    
    // 1. Test I2C bus communication
    debug_print("1. Testing I2C bus communication...\r\n");
    
    // Test both possible addresses
    uint8_t addresses[] = {0x76, 0x77};
    const char* addr_names[] = {"0x76", "0x77"};
    int device_found = 0;
    
    for (int i = 0; i < 2; i++) {
        snprintf(debug_msg, sizeof(debug_msg), "   Testing address %s...\r\n", addr_names[i]);
        debug_print(debug_msg);
        
        // Test device ready
        status = HAL_I2C_IsDeviceReady(&hi2c1, addresses[i] << 1, 3, 1000);
        if (status == HAL_OK) {
            snprintf(debug_msg, sizeof(debug_msg), "   ✓ Device responds at %s\r\n", addr_names[i]);
            debug_print(debug_msg);
            device_found = 1;
            
            // Try to read chip ID
            status = HAL_I2C_Mem_Read(&hi2c1, addresses[i] << 1, 0xD0, 
                                      I2C_MEMADD_SIZE_8BIT, &chip_id, 1, 1000);
            
            if (status == HAL_OK) {
                snprintf(debug_msg, sizeof(debug_msg), "   ✓ Chip ID: 0x%02X (Expected: 0x61)\r\n", chip_id);
                debug_print(debug_msg);
                
                if (chip_id == 0x61) {
                    snprintf(debug_msg, sizeof(debug_msg), "   ✓ Valid BME680 chip ID found!\r\n");
                    debug_print(debug_msg);
                } else {
                    snprintf(debug_msg, sizeof(debug_msg), "   ✗ Wrong chip ID - not a BME680\r\n");
                    debug_print(debug_msg);
                }
            } else {
                snprintf(debug_msg, sizeof(debug_msg), "   ✗ Failed to read chip ID\r\n");
                debug_print(debug_msg);
            }
        } else {
            snprintf(debug_msg, sizeof(debug_msg), "   ✗ No device at %s\r\n", addr_names[i]);
            debug_print(debug_msg);
        }
    }
    
    if (!device_found) {
        debug_print("   ✗ No I2C devices found - check wiring!\r\n");
        debug_print("   Wiring check:\r\n");
        debug_print("     - PA9 (SCL) → BME680 SCL\r\n");
        debug_print("     - PA10 (SDA) → BME680 SDA\r\n");
        debug_print("     - 3.3V → BME680 VCC\r\n");
        debug_print("     - GND → BME680 GND\r\n");
        debug_print("     - Add 4.7kΩ pull-up resistors to SCL and SDA\r\n");
        return;
    }
    
    // 2. Test register access
    debug_print("\r\n2. Testing register access...\r\n");
    
    // Read multiple registers to test communication
    status = HAL_I2C_Mem_Read(&hi2c1, 0x76 << 1, 0xD0, 
                              I2C_MEMADD_SIZE_8BIT, raw_data, 8, 1000);
    
    if (status == HAL_OK) {
        snprintf(debug_msg, sizeof(debug_msg), 
                 "   ✓ Register read successful: 0xD0=0x%02X, 0xD1=0x%02X, 0xD2=0x%02X\r\n",
                 raw_data[0], raw_data[1], raw_data[2]);
        debug_print(debug_msg);
    } else {
        snprintf(debug_msg, sizeof(debug_msg), "   ✗ Register read failed: %d\r\n", status);
        debug_print(debug_msg);
    }
    
    // 3. Test sensor initialization
    debug_print("\r\n3. Testing sensor initialization...\r\n");
    
    int8_t init_result = bme680_init_sensor();
    snprintf(debug_msg, sizeof(debug_msg), "   Init result: %d\r\n", init_result);
    debug_print(debug_msg);
    
    if (init_result == BME68X_OK) {
        debug_print("   ✓ Sensor initialization successful\r\n");
    } else {
        debug_print("   ✗ Sensor initialization failed\r\n");
        debug_print("   Possible causes:\r\n");
        debug_print("     - Wrong I2C address\r\n");
        debug_print("     - Power supply issues\r\n");
        debug_print("     - Communication timing issues\r\n");
    }
    
    // 4. Test sensor data reading
    debug_print("\r\n4. Testing sensor data reading...\r\n");
    
    struct bme68x_data sensor_data;
    int8_t read_result = bme680_read_sensor_data(&sensor_data);
    snprintf(debug_msg, sizeof(debug_msg), "   Read result: %d\r\n", read_result);
    debug_print(debug_msg);
    
    if (read_result == BME68X_OK) {
        debug_print("   ✓ Sensor data read successful\r\n");
        
        // Check if values are reasonable
        snprintf(debug_msg, sizeof(debug_msg), 
                 "   Raw values - Temp: %.6f, Press: %.6f, Hum: %.6f\r\n",
                 sensor_data.temperature, sensor_data.pressure, sensor_data.humidity);
        debug_print(debug_msg);
        
        // Check for zero or invalid values
        if (sensor_data.temperature == 0.0f && sensor_data.pressure == 0.0f && sensor_data.humidity == 0.0f) {
            debug_print("   ⚠ All values are zero - possible library issue\r\n");
        } else if (isnan(sensor_data.temperature) || isnan(sensor_data.pressure) || isnan(sensor_data.humidity)) {
            debug_print("   ⚠ Invalid values detected (NaN)\r\n");
        } else {
            debug_print("   ✓ Values appear valid\r\n");
        }
    } else {
        debug_print("   ✗ Sensor data read failed\r\n");
    }
    
    // 5. Test raw register reading
    debug_print("\r\n5. Testing raw register reading...\r\n");
    
    // Read temperature, pressure, and humidity registers
    status = HAL_I2C_Mem_Read(&hi2c1, 0x76 << 1, 0x22, 
                              I2C_MEMADD_SIZE_8BIT, raw_data, 8, 1000);
    
    if (status == HAL_OK) {
        uint32_t temp_raw = ((uint32_t)raw_data[0] << 12) | ((uint32_t)raw_data[1] << 4) | ((uint32_t)raw_data[2] >> 4);
        uint32_t press_raw = ((uint32_t)raw_data[3] << 12) | ((uint32_t)raw_data[4] << 4) | ((uint32_t)raw_data[5] >> 4);
        uint16_t hum_raw = ((uint16_t)raw_data[6] << 8) | raw_data[7];
        
        snprintf(debug_msg, sizeof(debug_msg), 
                 "   Raw ADC values - Temp: %lu, Press: %lu, Hum: %u\r\n",
                 temp_raw, press_raw, hum_raw);
        debug_print(debug_msg);
        
        // Check if raw values are reasonable (not all zeros)
        if (temp_raw == 0 && press_raw == 0 && hum_raw == 0) {
            debug_print("   ⚠ All raw ADC values are zero - sensor may not be measuring\r\n");
        } else {
            debug_print("   ✓ Raw ADC values appear valid\r\n");
        }
    } else {
        snprintf(debug_msg, sizeof(debug_msg), "   ✗ Raw register read failed: %d\r\n", status);
        debug_print(debug_msg);
    }
    
    // 6. Test calibration data
    debug_print("\r\n6. Testing calibration data...\r\n");
    
    uint8_t calib_data[32];
    status = HAL_I2C_Mem_Read(&hi2c1, 0x76 << 1, 0xE1, 
                              I2C_MEMADD_SIZE_8BIT, calib_data, 32, 1000);
    
    if (status == HAL_OK) {
        debug_print("   ✓ Calibration data read successful\r\n");
        
        // Check if calibration data looks reasonable (not all zeros or 0xFF)
        int valid_calib = 0;
        for (int i = 0; i < 32; i++) {
            if (calib_data[i] != 0 && calib_data[i] != 0xFF) {
                valid_calib = 1;
                break;
            }
        }
        
        if (valid_calib) {
            debug_print("   ✓ Calibration data appears valid\r\n");
        } else {
            debug_print("   ⚠ Calibration data may be invalid (all zeros or 0xFF)\r\n");
        }
    } else {
        snprintf(debug_msg, sizeof(debug_msg), "   ✗ Calibration data read failed: %d\r\n", status);
        debug_print(debug_msg);
    }
    
    debug_print("\r\n=== DIAGNOSTIC COMPLETE ===\r\n");
    debug_print("Summary:\r\n");
    debug_print("- If I2C scan failed: Check wiring and power supply\r\n");
    debug_print("- If chip ID wrong: Wrong sensor or address\r\n");
    debug_print("- If init failed: Check sensor configuration\r\n");
    debug_print("- If read failed: Check sensor mode and timing\r\n");
    debug_print("- If values zero: Check sensor operation mode\r\n");
    debug_print("- If values invalid: Check library compatibility\r\n");
} 