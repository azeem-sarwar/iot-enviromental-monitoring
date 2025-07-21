#include "bme680/bme680_interface.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart4;

// BME680 device structure
struct bme68x_dev bme680_dev;

// I2C bus scanner function
void i2c_scan_bus(void) {
    uint8_t found_devices = 0;
    
    for (uint8_t addr = 1; addr < 128; addr++) {
        HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(&hi2c1, addr << 1, 2, 100);
        if (status == HAL_OK) {
            found_devices++;
        }
    }
}

// Enhanced I2C read function for BME680
int8_t bme680_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    HAL_StatusTypeDef status;
    
    // Read data from BME680 using I2C
    status = HAL_I2C_Mem_Read(&hi2c1, BME68X_I2C_ADDR_LOW << 1, reg_addr, 
                              I2C_MEMADD_SIZE_8BIT, reg_data, len, 1000);
    
    if (status == HAL_OK) {
        return BME68X_OK;
    } else {
        return BME68X_E_COM_FAIL;
    }
}

// Enhanced I2C write function for BME680
int8_t bme680_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    HAL_StatusTypeDef status;
    
    // Write data to BME680 using I2C
    status = HAL_I2C_Mem_Write(&hi2c1, BME68X_I2C_ADDR_LOW << 1, reg_addr, 
                               I2C_MEMADD_SIZE_8BIT, (uint8_t*)reg_data, len, 1000);
    
    if (status == HAL_OK) {
        return BME68X_OK;
    } else {
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
    
    // Try both possible addresses
    uint8_t addresses[] = {BME68X_I2C_ADDR_LOW, BME68X_I2C_ADDR_HIGH};
    
    for (int i = 0; i < 2; i++) {
        // First check if device responds
        status = HAL_I2C_IsDeviceReady(&hi2c1, addresses[i] << 1, 3, 1000);
        if (status == HAL_OK) {
            // Try to read chip ID
            status = HAL_I2C_Mem_Read(&hi2c1, addresses[i] << 1, BME68X_REG_CHIP_ID, 
                                      I2C_MEMADD_SIZE_8BIT, &chip_id, 1, 1000);
            
            if (status == HAL_OK) {
                if (chip_id == BME68X_CHIP_ID) {
                    return BME68X_OK;
                }
            }
        }
    }
    
    return BME68X_E_DEV_NOT_FOUND;
}

// Enhanced sensor initialization
int8_t bme680_init_sensor(void)
{
    int8_t rslt;
    
    // Initialize device structure
    bme680_dev.intf = BME68X_I2C_INTF;
    bme680_dev.read = bme680_i2c_read;
    bme680_dev.write = bme680_i2c_write;
    bme680_dev.delay_us = bme680_delay_us;
    bme680_dev.intf_ptr = NULL;
    bme680_dev.amb_temp = 25;
    
    // Initialize the sensor
    rslt = bme68x_init(&bme680_dev);
    
    if (rslt == BME68X_OK) {
        // Configure sensor settings
        struct bme68x_conf conf;
        conf.os_hum = BME68X_OS_1X;
        conf.os_pres = BME68X_OS_1X;
        conf.os_temp = BME68X_OS_1X;
        conf.filter = BME68X_FILTER_OFF;
        conf.odr = BME68X_ODR_NONE;
        
        rslt = bme68x_set_conf(&conf, &bme680_dev);
        
        if (rslt == BME68X_OK) {
            // Disable gas sensor for faster readings
            struct bme68x_heatr_conf heatr_conf;
            heatr_conf.enable = BME68X_DISABLE;
            rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme680_dev);
        }
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
    
    // Set operation mode to forced mode
    rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &bme680_dev);
    
    if (rslt == BME68X_OK) {
        // Wait for measurement to complete
        uint32_t del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, NULL, &bme680_dev);
        
        bme680_dev.delay_us(del_period, bme680_dev.intf_ptr);
        
        // Read the data
        rslt = bme68x_get_data(BME68X_FORCED_MODE, data, &n_data, &bme680_dev);
        
        if (rslt == BME68X_OK && n_data > 0) {
            
            // Check validity of each value
            if (is_float_invalid(data->temperature) || is_float_invalid(data->pressure) || is_float_invalid(data->humidity)) {
                return BME68X_E_INVALID_LENGTH;
            }
            
            // Check if values are valid (not NaN or infinite)
            if (is_float_invalid(data->temperature) || is_float_invalid(data->pressure) || is_float_invalid(data->humidity)) {
                return BME68X_E_INVALID_LENGTH;
            }
            
            // Check if values are within reasonable ranges
            if (data->temperature < -40.0f || data->temperature > 85.0f ||
                data->pressure < 30000.0f || data->pressure > 125000.0f ||
                data->humidity < 0.0f || data->humidity > 100.0f) {
                return BME68X_E_INVALID_LENGTH;
            }
            
            // Apply temperature offset correction if needed
            // The BME680 might have a factory offset that needs correction
            float temp_offset = -9.5f; // Adjust this value based on your testing
            data->temperature += temp_offset;
        } else {
            return BME68X_E_INVALID_LENGTH;
        }
    } else {
        return BME68X_E_COM_FAIL;
    }
    
    return rslt;
}

// Print sensor data
void bme680_print_sensor_data(struct bme68x_data *data)
{
    // Function kept for compatibility but no debug output
}

// Check BME680 calibration data
void bme680_check_calibration_data(void)
{
    uint8_t calib_data[41];
    
    // Read calibration data from registers 0xE1 to 0xF0 and 0x8A to 0xA1
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c1, BME68X_I2C_ADDR_LOW << 1, 0xE1, 
                                                I2C_MEMADD_SIZE_8BIT, calib_data, 16, 1000);
    
    if (status == HAL_OK) {
        status = HAL_I2C_Mem_Read(&hi2c1, BME68X_I2C_ADDR_LOW << 1, 0x8A, 
                                  I2C_MEMADD_SIZE_8BIT, &calib_data[16], 25, 1000);
    }
}

// Read raw ADC values directly from BME680
void bme680_read_raw_adc_values(void)
{
    uint8_t raw_data[8];
    
    // Read raw temperature, pressure, and humidity ADC values
    // Temperature: registers 0x22-0x24
    // Pressure: registers 0x1F-0x21  
    // Humidity: registers 0x25-0x26
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c1, BME68X_I2C_ADDR_LOW << 1, 0x1F, 
                                                I2C_MEMADD_SIZE_8BIT, raw_data, 8, 1000);
    
    if (status == HAL_OK) {
        // Extract raw ADC values (for future use)
        // uint32_t raw_temp = ((uint32_t)raw_data[3] << 12) | ((uint32_t)raw_data[4] << 4) | ((uint32_t)raw_data[5] >> 4);
        // uint32_t raw_press = ((uint32_t)raw_data[0] << 12) | ((uint32_t)raw_data[1] << 4) | ((uint32_t)raw_data[2] >> 4);
        // uint16_t raw_hum = ((uint16_t)raw_data[6] << 8) | raw_data[7];
        
        // Also read the calibration data to understand the conversion
        uint8_t calib_temp[3];
        status = HAL_I2C_Mem_Read(&hi2c1, BME68X_I2C_ADDR_LOW << 1, 0xE1, 
                                  I2C_MEMADD_SIZE_8BIT, calib_temp, 3, 1000);
        
        if (status == HAL_OK) {
            // uint16_t T1 = (calib_temp[1] << 8) | calib_temp[0];
            // int16_t T2 = (int16_t)((calib_temp[3] << 8) | calib_temp[2]);
            // int8_t T3 = (int8_t)calib_temp[4];
        }
    }
}

// Simple direct register reading test
void bme680_read_raw_registers(void)
{
    uint8_t raw_data[8];
    
    // Read temperature, pressure, and humidity registers (0x22-0x26)
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c1, BME68X_I2C_ADDR_LOW << 1, 0x22, 
                                                I2C_MEMADD_SIZE_8BIT, raw_data, 8, 1000);
    
    if (status == HAL_OK) {
        // Calculate raw values (for future use)
        // uint32_t temp_raw = ((uint32_t)raw_data[0] << 12) | ((uint32_t)raw_data[1] << 4) | ((uint32_t)raw_data[2] >> 4);
        // uint32_t press_raw = ((uint32_t)raw_data[3] << 12) | ((uint32_t)raw_data[4] << 4) | ((uint32_t)raw_data[5] >> 4);
        // uint16_t hum_raw = ((uint16_t)raw_data[6] << 8) | raw_data[7];
    }
}

// Test BME680 sensor
void bme680_test_sensor(void)
{
    struct bme68x_data sensor_data;
    
    if (bme680_read_sensor_data(&sensor_data) == BME68X_OK) {
        // Sensor data read successfully
        // Values are already decoded by the BME680 library
    }
}

// Comprehensive BME680 diagnostic function
void bme680_comprehensive_diagnostic(void)
{
    HAL_StatusTypeDef status;
    uint8_t chip_id;
    uint8_t raw_data[32];
    
    // 1. Test I2C bus communication
    // Test both possible addresses
    uint8_t addresses[] = {0x76, 0x77};
    int device_found = 0;
    
    for (int i = 0; i < 2; i++) {
        // Test device ready
        status = HAL_I2C_IsDeviceReady(&hi2c1, addresses[i] << 1, 3, 1000);
        if (status == HAL_OK) {
            device_found = 1;
            
            // Try to read chip ID
            status = HAL_I2C_Mem_Read(&hi2c1, addresses[i] << 1, 0xD0, 
                                      I2C_MEMADD_SIZE_8BIT, &chip_id, 1, 1000);
            
            if (status == HAL_OK) {
                if (chip_id == 0x61) {
                    // Valid BME680 chip ID found
                }
            }
        }
    }
    
    if (!device_found) {
        return;
    }
    
    // 2. Test register access
    // Read multiple registers to test communication
    status = HAL_I2C_Mem_Read(&hi2c1, 0x76 << 1, 0xD0, 
                              I2C_MEMADD_SIZE_8BIT, raw_data, 8, 1000);
    
    // 3. Test sensor initialization
    bme680_init_sensor();
    
    // 4. Test sensor data reading
    struct bme68x_data sensor_data;
    int8_t read_result = bme680_read_sensor_data(&sensor_data);
    
    if (read_result == BME68X_OK) {
        // Check if values are reasonable
        // Check for zero or invalid values
        if (sensor_data.temperature == 0.0f && sensor_data.pressure == 0.0f && sensor_data.humidity == 0.0f) {
            // All values are zero - possible library issue
        } else if (isnan(sensor_data.temperature) || isnan(sensor_data.pressure) || isnan(sensor_data.humidity)) {
            // Invalid values detected (NaN)
        } else {
            // Values appear valid
        }
    }
    
    // 5. Test raw register reading
    // Read temperature, pressure, and humidity registers
    status = HAL_I2C_Mem_Read(&hi2c1, 0x76 << 1, 0x22, 
                              I2C_MEMADD_SIZE_8BIT, raw_data, 8, 1000);
    
    if (status == HAL_OK) {
        uint32_t temp_raw = ((uint32_t)raw_data[0] << 12) | ((uint32_t)raw_data[1] << 4) | ((uint32_t)raw_data[2] >> 4);
        uint32_t press_raw = ((uint32_t)raw_data[3] << 12) | ((uint32_t)raw_data[4] << 4) | ((uint32_t)raw_data[5] >> 4);
        uint16_t hum_raw = ((uint16_t)raw_data[6] << 8) | raw_data[7];
        
        // Check if raw values are reasonable (not all zeros)
        if (temp_raw == 0 && press_raw == 0 && hum_raw == 0) {
            // All raw ADC values are zero - sensor may not be measuring
        } else {
            // Raw ADC values appear valid
        }
    }
    
    // 6. Test calibration data
    uint8_t calib_data[32];
    status = HAL_I2C_Mem_Read(&hi2c1, 0x76 << 1, 0xE1, 
                              I2C_MEMADD_SIZE_8BIT, calib_data, 32, 1000);
    
    if (status == HAL_OK) {
        // Check if calibration data looks reasonable (not all zeros or 0xFF)
        int valid_calib = 0;
        for (int i = 0; i < 32; i++) {
            if (calib_data[i] != 0 && calib_data[i] != 0xFF) {
                valid_calib = 1;
                break;
            }
        }
        
        if (valid_calib) {
            // Calibration data appears valid
        } else {
            // Calibration data may be invalid (all zeros or 0xFF)
        }
    }
} 