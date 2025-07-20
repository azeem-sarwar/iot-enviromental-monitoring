#ifndef __COMMAND_INTERFACE_H__
#define __COMMAND_INTERFACE_H__

#include "stm32g0xx_hal.h"
#include "bme68x.h"

// Command buffer size
#define CMD_BUFFER_SIZE 64

// Function prototypes
void command_interface_init(void);
void command_interface_process(void);
void command_interface_show_help(void);
void command_interface_handle_command(char* command);
void command_interface_handle_command_usart3(char* command);
void command_interface_send_response(const char* response);
void command_interface_send_response_usart3(const char* response);

// Command handlers
void cmd_read_temperature(void);
void cmd_read_pressure(void);
void cmd_read_humidity(void);
void cmd_test_sensor(void);
void cmd_math_operation(char* command);

// USART3 Command handlers (clean version)
void cmd_read_temperature_usart3(void);
void cmd_read_pressure_usart3(void);
void cmd_read_humidity_usart3(void);
void cmd_test_sensor_usart3(void);
void cmd_math_operation_usart3(char* command);
void command_interface_show_help_usart3(void);

// LoRa Command handlers
void cmd_lora_broadcast(void);
void cmd_lora_broadcast_usart3(void);
void cmd_lora_config_usart3(void);
void cmd_lora_test_usart3(void);
void cmd_test_spi(void);
void cmd_test_spi_usart3(void);
void cmd_test_lora_init(void);
void cmd_test_lora_init_usart3(void);

#endif // __COMMAND_INTERFACE_H__ 