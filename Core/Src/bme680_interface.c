#include "bme680_interface.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

extern I2C_HandleTypeDef hi2c1;

// BME680 device structure
struct bme68x_dev bme680_dev;

// I2C read function for BME680
int8_t bme680_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    HAL_StatusTypeDef status;
    
    // Read data from BME680 using I2C
    status = HAL_I2C_Mem_Read(&hi2c1, BME68X_I2C_ADDR_LOW << 1, reg_addr, 
                              I2C_MEMADD_SIZE_8BIT, reg_data, len, HAL_MAX_DELAY);
    
    if (status == HAL_OK) {
        return BME68X_OK;
    } else {
        return BME68X_E_COM_FAIL;
    }
}

// I2C write function for BME680
int8_t bme680_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    HAL_StatusTypeDef status;
    
    // Write data to BME680 using I2C
    status = HAL_I2C_Mem_Write(&hi2c1, BME68X_I2C_ADDR_LOW << 1, reg_addr, 
                               I2C_MEMADD_SIZE_8BIT, (uint8_t*)reg_data, len, HAL_MAX_DELAY);
    
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

// Initialize BME680 sensor
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
            // Configure gas sensor (optional - for gas resistance measurement)
            struct bme68x_heatr_conf heatr_conf;
            heatr_conf.enable = BME68X_DISABLE; // Disable gas sensor for now
            heatr_conf.heatr_temp = 300;
            heatr_conf.heatr_dur = 100;
            
            rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme680_dev);
        }
    }
    
    // Debug output
    extern UART_HandleTypeDef huart2;
    char debug_msg[128];
    snprintf(debug_msg, sizeof(debug_msg), "BME680 Init Result: %d\r\n", rslt);
    HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
    
    return rslt;
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
    
    // Send via USART2 (you can modify this to use USART4 if needed)
    extern UART_HandleTypeDef huart2;
    HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}

// Test BME680 sensor
void bme680_test_sensor(void)
{
    struct bme68x_data sensor_data;
    extern UART_HandleTypeDef huart2;
    char test_msg[256];
    
    snprintf(test_msg, sizeof(test_msg), "Testing BME680 sensor...\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t*)test_msg, strlen(test_msg), HAL_MAX_DELAY);
    
    if (bme680_read_sensor_data(&sensor_data) == BME68X_OK) {
        snprintf(test_msg, sizeof(test_msg), 
                 "Test successful!\r\nTemperature: %.2f°C\r\nPressure: %.2f Pa\r\nHumidity: %.2f%%\r\n",
                 sensor_data.temperature, sensor_data.pressure, sensor_data.humidity);
    } else {
        snprintf(test_msg, sizeof(test_msg), "Test failed! Error reading sensor data.\r\n");
    }
    
    HAL_UART_Transmit(&huart2, (uint8_t*)test_msg, strlen(test_msg), HAL_MAX_DELAY);
}

// Check if BME680 sensor is present on I2C bus
int8_t bme680_check_sensor_presence(void)
{
    HAL_StatusTypeDef status;
    uint8_t chip_id;
    
    // Try to read chip ID from BME680
    status = HAL_I2C_Mem_Read(&hi2c1, BME68X_I2C_ADDR_LOW << 1, BME68X_REG_CHIP_ID, 
                              I2C_MEMADD_SIZE_8BIT, &chip_id, 1, 1000);
    
    if (status == HAL_OK && chip_id == BME68X_CHIP_ID) {
        return BME68X_OK;
    } else {
        return BME68X_E_DEV_NOT_FOUND;
    }
} 