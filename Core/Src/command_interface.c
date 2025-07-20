#include "command_interface.h"
#include "bme680_interface.h"
#include "lora_interface.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

// Global variables
static char cmd_buffer[CMD_BUFFER_SIZE];
static char cmd_buffer_usart3[CMD_BUFFER_SIZE];
static uint8_t cmd_index = 0;
static uint8_t cmd_index_usart3 = 0;
static uint8_t system_started = 0;
static uint8_t system_started_usart3 = 0;

// Initialize command interface
void command_interface_init(void)
{
    // Initialize USART2 command interface (debug)
    cmd_index = 0;
    system_started = 0;
    memset(cmd_buffer, 0, CMD_BUFFER_SIZE);
    
    // Initialize USART3 command interface (clean receiver)
    cmd_index_usart3 = 0;
    system_started_usart3 = 0;
    memset(cmd_buffer_usart3, 0, CMD_BUFFER_SIZE);
    
    // Send welcome message on USART2 (debug)
    command_interface_send_response("IoT Prototype System Ready (USART2 - Debug)\r\n");
    command_interface_send_response("Type 'start' to begin\r\n");
    
    // Send clean welcome message on USART3 (receiver)
    command_interface_send_response_usart3("IoT System Ready\r\n");
    command_interface_send_response_usart3("Type 'start' to begin\r\n");
}

// Process incoming commands
void command_interface_process(void)
{
    uint8_t rx_byte;
    
    // Check if data is available on USART2 (debug interface)
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
                
                // Process command (debug mode)
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
            // Echo character back to both USART2 and USART3
            HAL_UART_Transmit(&huart2, &rx_byte, 1, HAL_MAX_DELAY);
            HAL_UART_Transmit(&huart3, &rx_byte, 1, HAL_MAX_DELAY);
        }
    }
    
    // Check if data is available on USART3 (clean receiver)
    if (HAL_UART_Receive(&huart3, &rx_byte, 1, 10) == HAL_OK) {
        // Handle backspace
        if (rx_byte == '\b' || rx_byte == 127) {
            if (cmd_index_usart3 > 0) {
                cmd_index_usart3--;
                cmd_buffer_usart3[cmd_index_usart3] = '\0';
                command_interface_send_response_usart3("\b \b"); // Backspace, space, backspace
            }
        }
        // Handle enter key
        else if (rx_byte == '\r' || rx_byte == '\n') {
            if (cmd_index_usart3 > 0) {
                cmd_buffer_usart3[cmd_index_usart3] = '\0';
                command_interface_send_response_usart3("\r\n");
                
                // Process command (clean mode)
                command_interface_handle_command_usart3(cmd_buffer_usart3);
                
                // Reset buffer
                cmd_index_usart3 = 0;
                memset(cmd_buffer_usart3, 0, CMD_BUFFER_SIZE);
            }
            command_interface_send_response_usart3("> ");
        }
        // Handle regular characters
        else if (cmd_index_usart3 < CMD_BUFFER_SIZE - 1 && rx_byte >= 32 && rx_byte <= 126) {
            cmd_buffer_usart3[cmd_index_usart3++] = rx_byte;
            // Echo character back to both USART3 and USART2
            HAL_UART_Transmit(&huart3, &rx_byte, 1, HAL_MAX_DELAY);
            HAL_UART_Transmit(&huart2, &rx_byte, 1, HAL_MAX_DELAY);
        }
    }
}

// Show help menu (USART2 - debug)
void command_interface_show_help(void)
{
    command_interface_send_response("\r\n=== Available Commands (Debug Mode) ===\r\n");
    command_interface_send_response("Sensor Commands:\r\n");
    command_interface_send_response("  read temperature (rt) - Read temperature from BME680\r\n");
    command_interface_send_response("  read pressure (rp)    - Read pressure from BME680\r\n");
    command_interface_send_response("  read humidity (rh)    - Read humidity from BME680\r\n");
    command_interface_send_response("  test sensor (ts)      - Test BME680 sensor\r\n");
    command_interface_send_response("  raw registers (rr)    - Read raw BME680 registers\r\n");
    command_interface_send_response("  raw adc (ra)          - Read raw BME680 ADC values\r\n");
    command_interface_send_response("  calib data (cd)       - Check BME680 calibration data\r\n");
    command_interface_send_response("  bme diagnostic (bd)   - Run comprehensive BME680 diagnostic\r\n");
    command_interface_send_response("  scan i2c (si)         - Scan I2C bus for devices\r\n");
    command_interface_send_response("\r\nLoRa Commands:\r\n");
    command_interface_send_response("  lora broadcast (lb)   - Broadcast sensor data via SX1262 LoRa\r\n");
    command_interface_send_response("  lora config (lc)      - Show SX1262 LoRa configuration\r\n");
    command_interface_send_response("  lora test (lt)        - Test SX1262 LoRa transmission\r\n");
    command_interface_send_response("  lora scan (ls)        - Scan for LoRa signals (5s)\r\n");
    command_interface_send_response("  lora monitor (lm)     - Start continuous monitoring\r\n");
    command_interface_send_response("  lora stop (lst)       - Stop LoRa monitoring\r\n");
    command_interface_send_response("  lora rssi (lr)        - Get current RSSI\r\n");
    command_interface_send_response("  test spi (tspi)       - Test SPI communication\r\n");
    command_interface_send_response("  test lora (tl)        - Test LoRa module initialization\r\n");
    command_interface_send_response("\r\nMath Operations:\r\n");
    command_interface_send_response("  sum <num1> <num2>     - Add two numbers\r\n");
    command_interface_send_response("  sub <num1> <num2>     - Subtract num2 from num1\r\n");
    command_interface_send_response("  mul <num1> <num2>     - Multiply two numbers\r\n");
    command_interface_send_response("  div <num1> <num2>     - Divide num1 by num2\r\n");
    command_interface_send_response("\r\nSystem:\r\n");
    command_interface_send_response("  help                  - Show this help menu\r\n");
    command_interface_send_response("========================\r\n");
}

// Handle incoming commands (USART2 - debug mode)
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
    else if (strcmp(command, "bme diagnostic") == 0 || strcmp(command, "bd") == 0) {
        bme680_comprehensive_diagnostic();
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
    else if (strcmp(command, "test spi") == 0 || strcmp(command, "tspi") == 0) {
        cmd_test_spi();
    }
    else if (strcmp(command, "test lora") == 0 || strcmp(command, "tl") == 0) {
        cmd_test_lora_init();
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

// Send response via USART2 (debug) and also to USART3
void command_interface_send_response(const char* response)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)response, strlen(response), HAL_MAX_DELAY);
    // Also send to USART3 for dual communication
    HAL_UART_Transmit(&huart3, (uint8_t*)response, strlen(response), HAL_MAX_DELAY);
}

// Send response via USART3 (clean receiver) and also to USART2
void command_interface_send_response_usart3(const char* response)
{
    HAL_UART_Transmit(&huart3, (uint8_t*)response, strlen(response), HAL_MAX_DELAY);
    // Also send to USART2 for dual communication
    HAL_UART_Transmit(&huart2, (uint8_t*)response, strlen(response), HAL_MAX_DELAY);
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

// Show help menu for USART3 (clean version)
void command_interface_show_help_usart3(void)
{
    command_interface_send_response_usart3("\r\n=== Available Commands ===\r\n");
    command_interface_send_response_usart3("Sensor Commands:\r\n");
    command_interface_send_response_usart3("  rt - Read temperature\r\n");
    command_interface_send_response_usart3("  rp - Read pressure\r\n");
    command_interface_send_response_usart3("  rh - Read humidity\r\n");
    command_interface_send_response_usart3("  ts - Test sensor\r\n");
    command_interface_send_response_usart3("\r\nLoRa Commands:\r\n");
    command_interface_send_response_usart3("  lb - Broadcast sensor data\r\n");
    command_interface_send_response_usart3("  lc - Show LoRa configuration\r\n");
    command_interface_send_response_usart3("  lt - Test LoRa transmission\r\n");
    command_interface_send_response_usart3("\r\nMath Operations:\r\n");
    command_interface_send_response_usart3("  sum <num1> <num2> - Add numbers\r\n");
    command_interface_send_response_usart3("  sub <num1> <num2> - Subtract numbers\r\n");
    command_interface_send_response_usart3("  mul <num1> <num2> - Multiply numbers\r\n");
    command_interface_send_response_usart3("  div <num1> <num2> - Divide numbers\r\n");
    command_interface_send_response_usart3("\r\nSystem:\r\n");
    command_interface_send_response_usart3("  help - Show this menu\r\n");
    command_interface_send_response_usart3("========================\r\n");
}

// Handle incoming commands for USART3 (clean version)
void command_interface_handle_command_usart3(char* command)
{
    char response[256];
    
    // Check if system is started
    if (!system_started_usart3) {
        if (strcmp(command, "start") == 0) {
            system_started_usart3 = 1;
            command_interface_send_response_usart3("System ready\r\n");
            command_interface_send_response_usart3("> ");
            return;
        } else {
            command_interface_send_response_usart3("Type 'start' to begin\r\n");
            return;
        }
    }
    
    // Parse commands (full and abbreviated versions)
    if (strcmp(command, "read temperature") == 0 || strcmp(command, "rt") == 0) {
        cmd_read_temperature_usart3();
    }
    else if (strcmp(command, "read pressure") == 0 || strcmp(command, "rp") == 0) {
        cmd_read_pressure_usart3();
    }
    else if (strcmp(command, "read humidity") == 0 || strcmp(command, "rh") == 0) {
        cmd_read_humidity_usart3();
    }
    else if (strcmp(command, "test sensor") == 0 || strcmp(command, "ts") == 0) {
        cmd_test_sensor_usart3();
    }
    else if (strcmp(command, "lora broadcast") == 0 || strcmp(command, "lb") == 0) {
        cmd_lora_broadcast_usart3();
    }
    else if (strcmp(command, "lora config") == 0 || strcmp(command, "lc") == 0) {
        cmd_lora_config_usart3();
    }
    else if (strcmp(command, "lora test") == 0 || strcmp(command, "lt") == 0) {
        cmd_lora_test_usart3();
    }
    else if (strncmp(command, "sum ", 4) == 0) {
        cmd_math_operation_usart3(command);
    }
    else if (strncmp(command, "sub ", 4) == 0) {
        cmd_math_operation_usart3(command);
    }
    else if (strncmp(command, "mul ", 4) == 0) {
        cmd_math_operation_usart3(command);
    }
    else if (strncmp(command, "div ", 4) == 0) {
        cmd_math_operation_usart3(command);
    }
    else if (strcmp(command, "help") == 0) {
        command_interface_show_help_usart3();
    }
    else {
        snprintf(response, sizeof(response), "Unknown command: %s\r\nType 'help' for available commands.\r\n", command);
        command_interface_send_response_usart3(response);
    }
}

// USART4 Command handlers

// Command handler for reading temperature (USART3 - clean)
void cmd_read_temperature_usart3(void)
{
    struct bme68x_data sensor_data;
    char response[128];
    
    // Check if sensor is available
    if (bme680_check_sensor_presence() != BME68X_OK) {
        command_interface_send_response_usart3("Error: Sensor not available\r\n");
        return;
    }
    
    if (bme680_read_sensor_data(&sensor_data) == BME68X_OK) {
        snprintf(response, sizeof(response), "Temperature: %.2f°C\r\n", sensor_data.temperature);
    } else {
        snprintf(response, sizeof(response), "Error reading temperature\r\n");
    }
    
    command_interface_send_response_usart3(response);
}

// Command handler for reading pressure (USART3 - clean)
void cmd_read_pressure_usart3(void)
{
    struct bme68x_data sensor_data;
    char response[128];
    
    // Check if sensor is available
    if (bme680_check_sensor_presence() != BME68X_OK) {
        command_interface_send_response_usart3("Error: Sensor not available\r\n");
        return;
    }
    
    if (bme680_read_sensor_data(&sensor_data) == BME68X_OK) {
        snprintf(response, sizeof(response), "Pressure: %.2f Pa\r\n", sensor_data.pressure);
    } else {
        snprintf(response, sizeof(response), "Error reading pressure\r\n");
    }
    
    command_interface_send_response_usart3(response);
}

// Command handler for reading humidity (USART3 - clean)
void cmd_read_humidity_usart3(void)
{
    struct bme68x_data sensor_data;
    char response[128];
    
    // Check if sensor is available
    if (bme680_check_sensor_presence() != BME68X_OK) {
        command_interface_send_response_usart3("Error: Sensor not available\r\n");
        return;
    }
    
    if (bme680_read_sensor_data(&sensor_data) == BME68X_OK) {
        snprintf(response, sizeof(response), "Humidity: %.2f%%\r\n", sensor_data.humidity);
    } else {
        snprintf(response, sizeof(response), "Error reading humidity\r\n");
    }
    
    command_interface_send_response_usart3(response);
}

// Command handler for testing sensor (USART3 - clean)
void cmd_test_sensor_usart3(void)
{
    struct bme68x_data sensor_data;
    char response[256];
    
    // Check if sensor is available
    if (bme680_check_sensor_presence() != BME68X_OK) {
        command_interface_send_response_usart3("Error: Sensor not available\r\n");
        return;
    }
    
    if (bme680_read_sensor_data(&sensor_data) == BME68X_OK) {
        snprintf(response, sizeof(response), 
                "Sensor Test Results:\r\n"
                "Temperature: %.2f°C\r\n"
                "Pressure: %.2f Pa\r\n"
                "Humidity: %.2f%%\r\n",
                sensor_data.temperature, sensor_data.pressure, sensor_data.humidity);
    } else {
        snprintf(response, sizeof(response), "Error reading sensor data\r\n");
    }
    
    command_interface_send_response_usart3(response);
}

// Command handler for LoRa broadcast (USART3 - clean)
void cmd_lora_broadcast_usart3(void)
{
    struct bme68x_data sensor_data;
    char response[256];
    
    // Check if sensor is available
    if (bme680_check_sensor_presence() != BME68X_OK) {
        command_interface_send_response_usart3("Error: Sensor not available\r\n");
        return;
    }
    
    // Read sensor data
    if (bme680_read_sensor_data(&sensor_data) != BME68X_OK) {
        command_interface_send_response_usart3("Error reading sensor data\r\n");
        return;
    }
    
    // Send via LoRa
    if (sx1262_send_sensor_data(sensor_data.temperature, sensor_data.pressure, sensor_data.humidity) == 0) {
        snprintf(response, sizeof(response), 
                "Data sent via LoRa:\r\n"
                "Temperature: %.2f°C\r\n"
                "Pressure: %.2f Pa\r\n"
                "Humidity: %.2f%%\r\n",
                sensor_data.temperature, sensor_data.pressure, sensor_data.humidity);
    } else {
        snprintf(response, sizeof(response), "Error sending data via LoRa\r\n");
    }
    
    command_interface_send_response_usart3(response);
}

// Command handler for LoRa config (USART3 - clean)
void cmd_lora_config_usart3(void)
{
    command_interface_send_response_usart3("LoRa Configuration:\r\n");
    command_interface_send_response_usart3("Frequency: 868 MHz\r\n");
    command_interface_send_response_usart3("Spreading Factor: SF7\r\n");
    command_interface_send_response_usart3("Bandwidth: 125 kHz\r\n");
    command_interface_send_response_usart3("TX Power: 14 dBm\r\n");
    command_interface_send_response_usart3("Coding Rate: 4/5\r\n");
}

// Command handler for LoRa test (USART3 - clean)
void cmd_lora_test_usart3(void)
{
    if (sx1262_test_transmission() == 0) {
        command_interface_send_response_usart3("LoRa transmission test successful\r\n");
    } else {
        command_interface_send_response_usart3("LoRa transmission test failed\r\n");
    }
}

// Command handler for math operations (USART3 - clean)
void cmd_math_operation_usart3(char* command)
{
    char* token;
    char* rest = command;
    char response[128];
    float num1, num2, result;
    
    // Get operation type
    token = strtok_r(rest, " ", &rest);
    if (token == NULL) {
        command_interface_send_response_usart3("Error: Invalid math operation\r\n");
        return;
    }
    
    // Get first number
    token = strtok_r(NULL, " ", &rest);
    if (token == NULL) {
        command_interface_send_response_usart3("Error: Missing first number\r\n");
        return;
    }
    num1 = atof(token);
    
    // Get second number
    token = strtok_r(NULL, " ", &rest);
    if (token == NULL) {
        command_interface_send_response_usart3("Error: Missing second number\r\n");
        return;
    }
    num2 = atof(token);
    
    // Perform calculation based on command
    if (strncmp(command, "sum ", 4) == 0) {
        result = num1 + num2;
        snprintf(response, sizeof(response), "%.2f + %.2f = %.2f\r\n", num1, num2, result);
    }
    else if (strncmp(command, "sub ", 4) == 0) {
        result = num1 - num2;
        snprintf(response, sizeof(response), "%.2f - %.2f = %.2f\r\n", num1, num2, result);
    }
    else if (strncmp(command, "mul ", 4) == 0) {
        result = num1 * num2;
        snprintf(response, sizeof(response), "%.2f * %.2f = %.2f\r\n", num1, num2, result);
    }
    else if (strncmp(command, "div ", 4) == 0) {
        if (num2 == 0) {
            command_interface_send_response_usart3("Error: Division by zero\r\n");
            return;
        }
        result = num1 / num2;
        snprintf(response, sizeof(response), "%.2f / %.2f = %.2f\r\n", num1, num2, result);
    }
    else {
        command_interface_send_response_usart3("Error: Unknown math operation\r\n");
        return;
    }
    
    command_interface_send_response_usart3(response);
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



// SPI test command handlers

// Test SPI communication (USART2)
void cmd_test_spi(void)
{
    command_interface_send_response("Testing SPI communication with SX1262...\r\n");
    
    if (sx1262_test_spi() == 0) {
        command_interface_send_response("✓ SPI communication test successful\r\n");
    } else {
        command_interface_send_response("✗ SPI communication test failed\r\n");
        command_interface_send_response("Check SPI connections and power supply\r\n");
    }
}

// Test SPI communication (USART3)
void cmd_test_spi_usart3(void)
{
    command_interface_send_response_usart3("Testing SPI communication with SX1262...\r\n");
    
    if (sx1262_test_spi() == 0) {
        command_interface_send_response_usart3("✓ SPI communication test successful\r\n");
    } else {
        command_interface_send_response_usart3("✗ SPI communication test failed\r\n");
        command_interface_send_response_usart3("Check SPI connections and power supply\r\n");
    }
}

// Test LoRa initialization (USART2)
void cmd_test_lora_init(void)
{
    command_interface_send_response("Testing LoRa module initialization...\r\n");
    
    // Test module detection
    command_interface_send_response("Step 1: Testing module detection...\r\n");
    if (sx1262_detect_module() == 0) {
        command_interface_send_response("✓ Module detected\r\n");
    } else {
        command_interface_send_response("✗ Module not detected\r\n");
        return;
    }
    
    // Test reset
    command_interface_send_response("Step 2: Testing module reset...\r\n");
    if (sx1262_reset() == 0) {
        command_interface_send_response("✓ Module reset successful\r\n");
    } else {
        command_interface_send_response("✗ Module reset failed\r\n");
        command_interface_send_response("Trying alternative initialization...\r\n");
    }
    
    // Test full initialization
    command_interface_send_response("Step 3: Testing full initialization...\r\n");
    if (sx1262_init() == 0) {
        command_interface_send_response("✓ LoRa module initialization successful\r\n");
    } else {
        command_interface_send_response("✗ LoRa module initialization failed\r\n");
    }
}

// Test LoRa initialization (USART3)
void cmd_test_lora_init_usart3(void)
{
    command_interface_send_response_usart3("Testing LoRa module initialization...\r\n");
    
    // Test module detection
    command_interface_send_response_usart3("Step 1: Testing module detection...\r\n");
    if (sx1262_detect_module() == 0) {
        command_interface_send_response_usart3("✓ Module detected\r\n");
    } else {
        command_interface_send_response_usart3("✗ Module not detected\r\n");
        return;
    }
    
    // Test reset
    command_interface_send_response_usart3("Step 2: Testing module reset...\r\n");
    if (sx1262_reset() == 0) {
        command_interface_send_response_usart3("✓ Module reset successful\r\n");
    } else {
        command_interface_send_response_usart3("✗ Module reset failed\r\n");
        command_interface_send_response_usart3("Trying alternative initialization...\r\n");
    }
    
    // Test full initialization
    command_interface_send_response_usart3("Step 3: Testing full initialization...\r\n");
    if (sx1262_init() == 0) {
        command_interface_send_response_usart3("✓ LoRa module initialization successful\r\n");
    } else {
        command_interface_send_response_usart3("✗ LoRa module initialization failed\r\n");
    }
}