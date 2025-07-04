#include "command_interface.h"
#include "bme680_interface.h"
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
    command_interface_send_response("read temperature - Read temperature from BME680\r\n");
    command_interface_send_response("read pressure   - Read pressure from BME680\r\n");
    command_interface_send_response("read humidity   - Read humidity from BME680\r\n");
    command_interface_send_response("test sensor     - Test BME680 sensor\r\n");
    command_interface_send_response("sum <num1> <num2> - Add two numbers\r\n");
    command_interface_send_response("sub <num1> <num2> - Subtract num2 from num1\r\n");
    command_interface_send_response("mul <num1> <num2> - Multiply two numbers\r\n");
    command_interface_send_response("div <num1> <num2> - Divide num1 by num2\r\n");
    command_interface_send_response("help            - Show this help menu\r\n");
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
    
    // Parse commands
    if (strcmp(command, "read temperature") == 0) {
        cmd_read_temperature();
    }
    else if (strcmp(command, "read pressure") == 0) {
        cmd_read_pressure();
    }
    else if (strcmp(command, "read humidity") == 0) {
        cmd_read_humidity();
    }
    else if (strcmp(command, "test sensor") == 0) {
        cmd_test_sensor();
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
    
    // Check if sensor is available
    if (bme680_check_sensor_presence() != BME68X_OK) {
        snprintf(response, sizeof(response), "Error: BME680 sensor not available\r\n");
        command_interface_send_response(response);
        return;
    }
    
    if (bme680_read_sensor_data(&sensor_data) == BME68X_OK) {
        snprintf(response, sizeof(response), "Temperature: %.2f°C\r\n", sensor_data.temperature);
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
        snprintf(response, sizeof(response), "Pressure: %.2f Pa\r\n", sensor_data.pressure);
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
        snprintf(response, sizeof(response), "Humidity: %.2f%%\r\n", sensor_data.humidity);
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
    command_interface_send_response_usart4("read temperature - Read temperature from BME680\r\n");
    command_interface_send_response_usart4("read pressure   - Read pressure from BME680\r\n");
    command_interface_send_response_usart4("read humidity   - Read humidity from BME680\r\n");
    command_interface_send_response_usart4("test sensor     - Test BME680 sensor\r\n");
    command_interface_send_response_usart4("sum <num1> <num2> - Add two numbers\r\n");
    command_interface_send_response_usart4("sub <num1> <num2> - Subtract num2 from num1\r\n");
    command_interface_send_response_usart4("mul <num1> <num2> - Multiply two numbers\r\n");
    command_interface_send_response_usart4("div <num1> <num2> - Divide num1 by num2\r\n");
    command_interface_send_response_usart4("help            - Show this help menu\r\n");
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
    
    // Parse commands
    if (strcmp(command, "read temperature") == 0) {
        cmd_read_temperature_usart4();
    }
    else if (strcmp(command, "read pressure") == 0) {
        cmd_read_pressure_usart4();
    }
    else if (strcmp(command, "read humidity") == 0) {
        cmd_read_humidity_usart4();
    }
    else if (strcmp(command, "test sensor") == 0) {
        cmd_test_sensor_usart4();
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