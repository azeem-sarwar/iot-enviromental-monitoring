#include "command_interface.h"
#include "bme680_interface.h"
#include "lora_interface.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart4;

// Global variables
static char cmd_buffer[CMD_BUFFER_SIZE];
static char cmd_buffer_usart4[CMD_BUFFER_SIZE];
static uint8_t cmd_index = 0;
static uint8_t cmd_index_usart4 = 0;
static uint8_t system_started = 0;
static uint8_t system_started_usart4 = 0;

// Initialize command interface
void command_interface_init(void)
{
    // Initialize USART2 command interface
    cmd_index = 0;
    system_started = 0;
    memset(cmd_buffer, 0, CMD_BUFFER_SIZE);
    
    // Initialize USART4 command interface
    cmd_index_usart4 = 0;
    system_started_usart4 = 0;
    memset(cmd_buffer_usart4, 0, CMD_BUFFER_SIZE);
    
    // Send welcome message on both UARTs
    command_interface_send_response("IoT Prototype System Ready (USART2)\r\n");
    command_interface_send_response("Type 'start' to begin\r\n");
    
    command_interface_send_response_usart4("IoT Prototype System Ready (USART4)\r\n");
    command_interface_send_response_usart4("Type 'start' to begin\r\n");
}

// Process incoming commands
void command_interface_process(void)
{
    uint8_t rx_byte;
    
    // Check if data is available on USART2
    if (HAL_UART_Receive(&huart2, &rx_byte, 1, 10) == HAL_OK) {
        // Handle backspace
        if (rx_byte == '\b' || rx_byte == 127) {
            if (cmd_index > 0) {
                cmd_index--;
                cmd_buffer[cmd_index] = '\0';
                command_interface_send_response("\b \b"); // Backspace, space, backspace
            }
        }
        // Handle enter key
        else if (rx_byte == '\r' || rx_byte == '\n') {
            if (cmd_index > 0) {
                cmd_buffer[cmd_index] = '\0';
                command_interface_send_response("\r\n");
                
                // Process command
                command_interface_handle_command(cmd_buffer);
                
                // Reset buffer
                cmd_index = 0;
                memset(cmd_buffer, 0, CMD_BUFFER_SIZE);
            }
            command_interface_send_response("> ");
        }
        // Handle regular characters
        else if (cmd_index < CMD_BUFFER_SIZE - 1 && rx_byte >= 32 && rx_byte <= 126) {
            cmd_buffer[cmd_index++] = rx_byte;
            // Echo character back
            HAL_UART_Transmit(&huart2, &rx_byte, 1, HAL_MAX_DELAY);
        }
    }
    
    // Check if data is available on USART4
    if (HAL_UART_Receive(&huart4, &rx_byte, 1, 10) == HAL_OK) {
        // Handle backspace
        if (rx_byte == '\b' || rx_byte == 127) {
            if (cmd_index_usart4 > 0) {
                cmd_index_usart4--;
                cmd_buffer_usart4[cmd_index_usart4] = '\0';
                command_interface_send_response_usart4("\b \b"); // Backspace, space, backspace
            }
        }
        // Handle enter key
        else if (rx_byte == '\r' || rx_byte == '\n') {
            if (cmd_index_usart4 > 0) {
                cmd_buffer_usart4[cmd_index_usart4] = '\0';
                command_interface_send_response_usart4("\r\n");
                
                // Process command
                command_interface_handle_command_usart4(cmd_buffer_usart4);
                
                // Reset buffer
                cmd_index_usart4 = 0;
                memset(cmd_buffer_usart4, 0, CMD_BUFFER_SIZE);
            }
            command_interface_send_response_usart4("> ");
        }
        // Handle regular characters
        else if (cmd_index_usart4 < CMD_BUFFER_SIZE - 1 && rx_byte >= 32 && rx_byte <= 126) {
            cmd_buffer_usart4[cmd_index_usart4++] = rx_byte;
            // Echo character back
            HAL_UART_Transmit(&huart4, &rx_byte, 1, HAL_MAX_DELAY);
        }
    }
}

// Show help menu
void command_interface_show_help(void)
{
    command_interface_send_response("\r\n=== Available Commands ===\r\n");
    command_interface_send_response("Sensor Commands:\r\n");
    command_interface_send_response("  read temperature (rt) - Read temperature from BME680\r\n");
    command_interface_send_response("  read pressure (rp)    - Read pressure from BME680\r\n");
    command_interface_send_response("  read humidity (rh)    - Read humidity from BME680\r\n");
    command_interface_send_response("  test sensor (ts)      - Test BME680 sensor\r\n");
    command_interface_send_response("  raw registers (rr)    - Read raw BME680 registers\r\n");
    command_interface_send_response("  raw adc (ra)          - Read raw BME680 ADC values\r\n");
    command_interface_send_response("  calib data (cd)       - Check BME680 calibration data\r\n");
    command_interface_send_response("  scan i2c (si)         - Scan I2C bus for devices\r\n");
    command_interface_send_response("\r\nLoRa Commands:\r\n");
    command_interface_send_response("  lora broadcast (lb)   - Broadcast sensor data via LoRa\r\n");
    command_interface_send_response("  lora config (lc)      - Show LoRa configuration\r\n");
    command_interface_send_response("  lora test (lt)        - Test LoRa transmission\r\n");
    command_interface_send_response("  lora scan (ls)        - Scan for LoRa signals (5s)\r\n");
    command_interface_send_response("  lora monitor (lm)     - Start continuous monitoring\r\n");
    command_interface_send_response("  lora stop (lst)       - Stop LoRa monitoring\r\n");
    command_interface_send_response("  lora rssi (lr)        - Get current RSSI\r\n");
    command_interface_send_response("\r\nMath Operations:\r\n");
    command_interface_send_response("  sum <num1> <num2>     - Add two numbers\r\n");
    command_interface_send_response("  sub <num1> <num2>     - Subtract num2 from num1\r\n");
    command_interface_send_response("  mul <num1> <num2>     - Multiply two numbers\r\n");
    command_interface_send_response("  div <num1> <num2>     - Divide num1 by num2\r\n");
    command_interface_send_response("\r\nSystem:\r\n");
    command_interface_send_response("  help                  - Show this help menu\r\n");
    command_interface_send_response("========================\r\n");
}

// Handle incoming commands
void command_interface_handle_command(char* command)
{
    char response[256];
    
    // Check if system is started
    if (!system_started) {
        if (strcmp(command, "start") == 0) {
            system_started = 1;
            command_interface_send_response("System started! Type 'help' for available commands.\r\n");
            command_interface_send_response("> ");
            return;
        } else {
            command_interface_send_response("Please type 'start' to begin.\r\n");
            return;
        }
    }
    
    // Parse commands (full and abbreviated versions)
    if (strcmp(command, "read temperature") == 0 || strcmp(command, "rt") == 0) {
        cmd_read_temperature();
    }
    else if (strcmp(command, "read pressure") == 0 || strcmp(command, "rp") == 0) {
        cmd_read_pressure();
    }
    else if (strcmp(command, "read humidity") == 0 || strcmp(command, "rh") == 0) {
        cmd_read_humidity();
    }
    else if (strcmp(command, "test sensor") == 0 || strcmp(command, "ts") == 0) {
        cmd_test_sensor();
    }
    else if (strcmp(command, "raw registers") == 0 || strcmp(command, "rr") == 0) {
        bme680_read_raw_registers();
    }
    else if (strcmp(command, "raw adc") == 0 || strcmp(command, "ra") == 0) {
        bme680_read_raw_adc_values();
    }
    else if (strcmp(command, "calib data") == 0 || strcmp(command, "cd") == 0) {
        bme680_check_calibration_data();
    }
    else if (strcmp(command, "scan i2c") == 0 || strcmp(command, "si") == 0) {
        i2c_scan_bus();
    }
    else if (strcmp(command, "lora broadcast") == 0 || strcmp(command, "lb") == 0) {
        cmd_lora_broadcast();
    }
    else if (strcmp(command, "lora config") == 0 || strcmp(command, "lc") == 0) {
        lora_print_config();
    }
    else if (strcmp(command, "lora test") == 0 || strcmp(command, "lt") == 0) {
        lora_test_transmission();
    }
    else if (strcmp(command, "lora scan") == 0 || strcmp(command, "ls") == 0) {
        lora_scan_signals(5000); // 5 second scan
    }
    else if (strcmp(command, "lora monitor") == 0 || strcmp(command, "lm") == 0) {
        lora_start_monitoring();
    }
    else if (strcmp(command, "lora stop") == 0 || strcmp(command, "lst") == 0) {
        lora_stop_monitoring();
    }
    else if (strcmp(command, "lora rssi") == 0 || strcmp(command, "lr") == 0) {
        lora_get_rssi();
    }
    else if (strncmp(command, "sum ", 4) == 0) {
        cmd_math_operation(command);
    }
    else if (strncmp(command, "sub ", 4) == 0) {
        cmd_math_operation(command);
    }
    else if (strncmp(command, "mul ", 4) == 0) {
        cmd_math_operation(command);
    }
    else if (strncmp(command, "div ", 4) == 0) {
        cmd_math_operation(command);
    }
    else if (strcmp(command, "help") == 0) {
        command_interface_show_help();
    }
    else {
        snprintf(response, sizeof(response), "Unknown command: %s\r\nType 'help' for available commands.\r\n", command);
        command_interface_send_response(response);
    }
}

// Send response via USART2
void command_interface_send_response(const char* response)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)response, strlen(response), HAL_MAX_DELAY);
}

// Send response via USART4
void command_interface_send_response_usart4(const char* response)
{
    HAL_UART_Transmit(&huart4, (uint8_t*)response, strlen(response), HAL_MAX_DELAY);
}

// Command handler for reading temperature
void cmd_read_temperature(void)
{
    struct bme68x_data sensor_data;
    char response[128];
    char debug_msg[128];
    
    // Check if sensor is available
    if (bme680_check_sensor_presence() != BME68X_OK) {
        snprintf(response, sizeof(response), "Error: BME680 sensor not available\r\n");
        command_interface_send_response(response);
        return;
    }
    
    if (bme680_read_sensor_data(&sensor_data) == BME68X_OK) {
        // Decode the IEEE 754 value from memory
        uint32_t temp_mem = *(uint32_t*)&sensor_data.temperature;
        float temp_decoded = decode_ieee754(temp_mem);
        
        // Direct IEEE 754 calculation for temperature
        uint32_t temp_sign = (temp_mem >> 31) & 0x1;
        uint32_t temp_exp = (temp_mem >> 23) & 0xFF;
        uint32_t temp_mant = temp_mem & 0x7FFFFF;
        
        if (temp_exp != 0 && temp_exp != 0xFF) {
            int temp_exp_val = (int)temp_exp - 127;
            float temp_result = 1.0f;
            
            // Calculate mantissa
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
            
            temp_decoded = temp_result;
        }
        
        // Debug: Print the raw value before formatting
        snprintf(debug_msg, sizeof(debug_msg), "DEBUG: Raw temperature value: %.6f\r\n", temp_decoded);
        command_interface_send_response(debug_msg);
        
        snprintf(response, sizeof(response), "Temperature: %.2f°C\r\n", temp_decoded);
        
        // Debug: Print the formatted string
        snprintf(debug_msg, sizeof(debug_msg), "DEBUG: Formatted string: '%.80s'\r\n", response);
        command_interface_send_response(debug_msg);
    } else {
        snprintf(response, sizeof(response), "Error reading temperature from BME680\r\n");
    }
    
    command_interface_send_response(response);
}

// Command handler for reading pressure
void cmd_read_pressure(void)
{
    struct bme68x_data sensor_data;
    char response[128];
    
    // Check if sensor is available
    if (bme680_check_sensor_presence() != BME68X_OK) {
        snprintf(response, sizeof(response), "Error: BME680 sensor not available\r\n");
        command_interface_send_response(response);
        return;
    }
    
    if (bme680_read_sensor_data(&sensor_data) == BME68X_OK) {
        // Decode the IEEE 754 value from memory
        uint32_t press_mem = *(uint32_t*)&sensor_data.pressure;
        uint32_t press_sign = (press_mem >> 31) & 0x1;
        uint32_t press_exp = (press_mem >> 23) & 0xFF;
        uint32_t press_mant = press_mem & 0x7FFFFF;
        
        float press_decoded = 0.0f;
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
        
        snprintf(response, sizeof(response), "Pressure: %.2f Pa\r\n", press_decoded);
    } else {
        snprintf(response, sizeof(response), "Error reading pressure from BME680\r\n");
    }
    
    command_interface_send_response(response);
}

// Command handler for reading humidity
void cmd_read_humidity(void)
{
    struct bme68x_data sensor_data;
    char response[128];
    
    // Check if sensor is available
    if (bme680_check_sensor_presence() != BME68X_OK) {
        snprintf(response, sizeof(response), "Error: BME680 sensor not available\r\n");
        command_interface_send_response(response);
        return;
    }
    
    if (bme680_read_sensor_data(&sensor_data) == BME68X_OK) {
        // Decode the IEEE 754 value from memory
        uint32_t hum_mem = *(uint32_t*)&sensor_data.humidity;
        uint32_t hum_sign = (hum_mem >> 31) & 0x1;
        uint32_t hum_exp = (hum_mem >> 23) & 0xFF;
        uint32_t hum_mant = hum_mem & 0x7FFFFF;
        
        float hum_decoded = 0.0f;
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
        
        snprintf(response, sizeof(response), "Humidity: %.2f%%\r\n", hum_decoded);
    } else {
        snprintf(response, sizeof(response), "Error reading humidity from BME680\r\n");
    }
    
    command_interface_send_response(response);
}

// Command handler for testing sensor
void cmd_test_sensor(void)
{
    // Check if sensor is available
    if (bme680_check_sensor_presence() != BME68X_OK) {
        command_interface_send_response("Error: BME680 sensor not available\r\n");
        return;
    }
    
    bme680_test_sensor();
}

// Command handler for mathematical operations
void cmd_math_operation(char* command)
{
    char* operation = strtok(command, " ");
    char* num1_str = strtok(NULL, " ");
    char* num2_str = strtok(NULL, " ");
    char response[128];
    
    if (num1_str == NULL || num2_str == NULL) {
        command_interface_send_response("Usage: <operation> <num1> <num2>\r\n");
        return;
    }
    
    float num1 = atof(num1_str);
    float num2 = atof(num2_str);
    float result;
    
    if (strcmp(operation, "sum") == 0) {
        result = num1 + num2;
        snprintf(response, sizeof(response), "%.2f + %.2f = %.2f\r\n", num1, num2, result);
    }
    else if (strcmp(operation, "sub") == 0) {
        result = num1 - num2;
        snprintf(response, sizeof(response), "%.2f - %.2f = %.2f\r\n", num1, num2, result);
    }
    else if (strcmp(operation, "mul") == 0) {
        result = num1 * num2;
        snprintf(response, sizeof(response), "%.2f * %.2f = %.2f\r\n", num1, num2, result);
    }
    else if (strcmp(operation, "div") == 0) {
        if (num2 != 0) {
            result = num1 / num2;
            snprintf(response, sizeof(response), "%.2f / %.2f = %.2f\r\n", num1, num2, result);
        } else {
            snprintf(response, sizeof(response), "Error: Division by zero\r\n");
        }
    }
    else {
        snprintf(response, sizeof(response), "Unknown operation: %s\r\n", operation);
    }
    
    command_interface_send_response(response);
}

// Show help menu for USART4
void command_interface_show_help_usart4(void)
{
    command_interface_send_response_usart4("\r\n=== Available Commands (USART4) ===\r\n");
    command_interface_send_response_usart4("Sensor Commands:\r\n");
    command_interface_send_response_usart4("  read temperature (rt) - Read temperature from BME680\r\n");
    command_interface_send_response_usart4("  read pressure (rp)    - Read pressure from BME680\r\n");
    command_interface_send_response_usart4("  read humidity (rh)    - Read humidity from BME680\r\n");
    command_interface_send_response_usart4("  test sensor (ts)      - Test BME680 sensor\r\n");
    command_interface_send_response_usart4("  raw registers (rr)    - Read raw BME680 registers\r\n");
    command_interface_send_response_usart4("  raw adc (ra)          - Read raw BME680 ADC values\r\n");
    command_interface_send_response_usart4("  calib data (cd)       - Check BME680 calibration data\r\n");
    command_interface_send_response_usart4("  scan i2c (si)         - Scan I2C bus for devices\r\n");
    command_interface_send_response_usart4("  lora broadcast (lb)   - Broadcast sensor data via LoRa\r\n");
    command_interface_send_response_usart4("  lora config (lc)      - Show LoRa configuration\r\n");
    command_interface_send_response_usart4("  lora test (lt)        - Test LoRa transmission\r\n");
    command_interface_send_response_usart4("\r\nMath Operations:\r\n");
    command_interface_send_response_usart4("  sum <num1> <num2>     - Add two numbers\r\n");
    command_interface_send_response_usart4("  sub <num1> <num2>     - Subtract num2 from num1\r\n");
    command_interface_send_response_usart4("  mul <num1> <num2>     - Multiply two numbers\r\n");
    command_interface_send_response_usart4("  div <num1> <num2>     - Divide num1 by num2\r\n");
    command_interface_send_response_usart4("\r\nSystem:\r\n");
    command_interface_send_response_usart4("  help                  - Show this help menu\r\n");
    command_interface_send_response_usart4("========================\r\n");
}

// Handle incoming commands for USART4
void command_interface_handle_command_usart4(char* command)
{
    char response[256];
    
    // Check if system is started
    if (!system_started_usart4) {
        if (strcmp(command, "start") == 0) {
            system_started_usart4 = 1;
            command_interface_send_response_usart4("System started! Type 'help' for available commands.\r\n");
            command_interface_send_response_usart4("> ");
            return;
        } else {
            command_interface_send_response_usart4("Please type 'start' to begin.\r\n");
            return;
        }
    }
    
    // Parse commands (full and abbreviated versions)
    if (strcmp(command, "read temperature") == 0 || strcmp(command, "rt") == 0) {
        cmd_read_temperature_usart4();
    }
    else if (strcmp(command, "read pressure") == 0 || strcmp(command, "rp") == 0) {
        cmd_read_pressure_usart4();
    }
    else if (strcmp(command, "read humidity") == 0 || strcmp(command, "rh") == 0) {
        cmd_read_humidity_usart4();
    }
    else if (strcmp(command, "test sensor") == 0 || strcmp(command, "ts") == 0) {
        cmd_test_sensor_usart4();
    }
    else if (strcmp(command, "raw registers") == 0 || strcmp(command, "rr") == 0) {
        bme680_read_raw_registers();
    }
    else if (strcmp(command, "raw adc") == 0 || strcmp(command, "ra") == 0) {
        bme680_read_raw_adc_values();
    }
    else if (strcmp(command, "calib data") == 0 || strcmp(command, "cd") == 0) {
        bme680_check_calibration_data();
    }
    else if (strcmp(command, "scan i2c") == 0 || strcmp(command, "si") == 0) {
        i2c_scan_bus();
    }
    else if (strcmp(command, "lora broadcast") == 0 || strcmp(command, "lb") == 0) {
        cmd_lora_broadcast_usart4();
    }
    else if (strcmp(command, "lora config") == 0 || strcmp(command, "lc") == 0) {
        lora_print_config();
    }
    else if (strcmp(command, "lora test") == 0 || strcmp(command, "lt") == 0) {
        lora_test_transmission();
    }
    else if (strcmp(command, "lora scan") == 0 || strcmp(command, "ls") == 0) {
        lora_scan_signals(5000); // 5 second scan
    }
    else if (strcmp(command, "lora monitor") == 0 || strcmp(command, "lm") == 0) {
        lora_start_monitoring();
    }
    else if (strcmp(command, "lora stop") == 0 || strcmp(command, "lst") == 0) {
        lora_stop_monitoring();
    }
    else if (strcmp(command, "lora rssi") == 0 || strcmp(command, "lr") == 0) {
        lora_get_rssi();
    }
    else if (strncmp(command, "sum ", 4) == 0) {
        cmd_math_operation_usart4(command);
    }
    else if (strncmp(command, "sub ", 4) == 0) {
        cmd_math_operation_usart4(command);
    }
    else if (strncmp(command, "mul ", 4) == 0) {
        cmd_math_operation_usart4(command);
    }
    else if (strncmp(command, "div ", 4) == 0) {
        cmd_math_operation_usart4(command);
    }
    else if (strcmp(command, "help") == 0) {
        command_interface_show_help_usart4();
    }
    else {
        snprintf(response, sizeof(response), "Unknown command: %s\r\nType 'help' for available commands.\r\n", command);
        command_interface_send_response_usart4(response);
    }
}

// USART4 Command handlers

// Command handler for reading temperature (USART4)
void cmd_read_temperature_usart4(void)
{
    struct bme68x_data sensor_data;
    char response[128];
    
    // Check if sensor is available
    if (bme680_check_sensor_presence() != BME68X_OK) {
        snprintf(response, sizeof(response), "Error: BME680 sensor not available\r\n");
        command_interface_send_response_usart4(response);
        return;
    }
    
    if (bme680_read_sensor_data(&sensor_data) == BME68X_OK) {
        snprintf(response, sizeof(response), "Temperature: %.2f°C\r\n", sensor_data.temperature);
    } else {
        snprintf(response, sizeof(response), "Error reading temperature from BME680\r\n");
    }
    
    command_interface_send_response_usart4(response);
}

// Command handler for reading pressure (USART4)
void cmd_read_pressure_usart4(void)
{
    struct bme68x_data sensor_data;
    char response[128];
    
    // Check if sensor is available
    if (bme680_check_sensor_presence() != BME68X_OK) {
        snprintf(response, sizeof(response), "Error: BME680 sensor not available\r\n");
        command_interface_send_response_usart4(response);
        return;
    }
    
    if (bme680_read_sensor_data(&sensor_data) == BME68X_OK) {
        snprintf(response, sizeof(response), "Pressure: %.2f Pa\r\n", sensor_data.pressure);
    } else {
        snprintf(response, sizeof(response), "Error reading pressure from BME680\r\n");
    }
    
    command_interface_send_response_usart4(response);
}

// Command handler for reading humidity (USART4)
void cmd_read_humidity_usart4(void)
{
    struct bme68x_data sensor_data;
    char response[128];
    
    // Check if sensor is available
    if (bme680_check_sensor_presence() != BME68X_OK) {
        snprintf(response, sizeof(response), "Error: BME680 sensor not available\r\n");
        command_interface_send_response_usart4(response);
        return;
    }
    
    if (bme680_read_sensor_data(&sensor_data) == BME68X_OK) {
        snprintf(response, sizeof(response), "Humidity: %.2f%%\r\n", sensor_data.humidity);
    } else {
        snprintf(response, sizeof(response), "Error reading humidity from BME680\r\n");
    }
    
    command_interface_send_response_usart4(response);
}

// Command handler for testing sensor (USART4)
void cmd_test_sensor_usart4(void)
{
    // Check if sensor is available
    if (bme680_check_sensor_presence() != BME68X_OK) {
        command_interface_send_response_usart4("Error: BME680 sensor not available\r\n");
        return;
    }
    
    // Create a custom test function for USART4
    struct bme68x_data sensor_data;
    char test_msg[256];
    
    snprintf(test_msg, sizeof(test_msg), "Testing BME680 sensor (USART4)...\r\n");
    command_interface_send_response_usart4(test_msg);
    
    if (bme680_read_sensor_data(&sensor_data) == BME68X_OK) {
        snprintf(test_msg, sizeof(test_msg), 
                 "Test successful!\r\nTemperature: %.2f°C\r\nPressure: %.2f Pa\r\nHumidity: %.2f%%\r\n",
                 sensor_data.temperature, sensor_data.pressure, sensor_data.humidity);
    } else {
        snprintf(test_msg, sizeof(test_msg), "Test failed! Error reading sensor data.\r\n");
    }
    
    command_interface_send_response_usart4(test_msg);
}

// Command handler for mathematical operations (USART4)
void cmd_math_operation_usart4(char* command)
{
    char* operation = strtok(command, " ");
    char* num1_str = strtok(NULL, " ");
    char* num2_str = strtok(NULL, " ");
    char response[128];
    
    if (num1_str == NULL || num2_str == NULL) {
        command_interface_send_response_usart4("Usage: <operation> <num1> <num2>\r\n");
        return;
    }
    
    float num1 = atof(num1_str);
    float num2 = atof(num2_str);
    float result;
    
    if (strcmp(operation, "sum") == 0) {
        result = num1 + num2;
        snprintf(response, sizeof(response), "%.2f + %.2f = %.2f\r\n", num1, num2, result);
    }
    else if (strcmp(operation, "sub") == 0) {
        result = num1 - num2;
        snprintf(response, sizeof(response), "%.2f - %.2f = %.2f\r\n", num1, num2, result);
    }
    else if (strcmp(operation, "mul") == 0) {
        result = num1 * num2;
        snprintf(response, sizeof(response), "%.2f * %.2f = %.2f\r\n", num1, num2, result);
    }
    else if (strcmp(operation, "div") == 0) {
        if (num2 != 0) {
            result = num1 / num2;
            snprintf(response, sizeof(response), "%.2f / %.2f = %.2f\r\n", num1, num2, result);
        } else {
            snprintf(response, sizeof(response), "Error: Division by zero\r\n");
        }
    }
    else {
        snprintf(response, sizeof(response), "Unknown operation: %s\r\n", operation);
    }
    
    command_interface_send_response_usart4(response);
}

// Command handler for LoRa broadcast (USART2)
void cmd_lora_broadcast(void)
{
    struct bme68x_data sensor_data;
    char response[128];
    
    // Check if sensor is available
    if (bme680_check_sensor_presence() != BME68X_OK) {
        snprintf(response, sizeof(response), "Error: BME680 sensor not available\r\n");
        command_interface_send_response(response);
        return;
    }
    
    // Read sensor data
    if (bme680_read_sensor_data(&sensor_data) == BME68X_OK) {
        snprintf(response, sizeof(response), 
                 "Broadcasting sensor data via LoRa...\r\nTemperature: %.2f°C, Pressure: %.2f Pa, Humidity: %.2f%%\r\n",
                 sensor_data.temperature, sensor_data.pressure, sensor_data.humidity);
        command_interface_send_response(response);
        
        // Send via LoRa
        if (lora_send_sensor_data(sensor_data.temperature, sensor_data.pressure, sensor_data.humidity) == 0) {
            command_interface_send_response("✓ LoRa broadcast successful\r\n");
        } else {
            command_interface_send_response("✗ LoRa broadcast failed\r\n");
        }
    } else {
        snprintf(response, sizeof(response), "Error reading sensor data for LoRa broadcast\r\n");
        command_interface_send_response(response);
    }
}

// Command handler for LoRa broadcast (USART4)
void cmd_lora_broadcast_usart4(void)
{
    struct bme68x_data sensor_data;
    char response[128];
    
    // Check if sensor is available
    if (bme680_check_sensor_presence() != BME68X_OK) {
        snprintf(response, sizeof(response), "Error: BME680 sensor not available\r\n");
        command_interface_send_response_usart4(response);
        return;
    }
    
    // Read sensor data
    if (bme680_read_sensor_data(&sensor_data) == BME68X_OK) {
        snprintf(response, sizeof(response), 
                 "Broadcasting sensor data via LoRa...\r\nTemperature: %.2f°C, Pressure: %.2f Pa, Humidity: %.2f%%\r\n",
                 sensor_data.temperature, sensor_data.pressure, sensor_data.humidity);
        command_interface_send_response_usart4(response);
        
        // Send via LoRa
        if (lora_send_sensor_data(sensor_data.temperature, sensor_data.pressure, sensor_data.humidity) == 0) {
            command_interface_send_response_usart4("✓ LoRa broadcast successful\r\n");
        } else {
            command_interface_send_response_usart4("✗ LoRa broadcast failed\r\n");
        }
    } else {
        snprintf(response, sizeof(response), "Error reading sensor data for LoRa broadcast\r\n");
        command_interface_send_response_usart4(response);
    }
}