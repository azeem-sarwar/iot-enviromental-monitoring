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
void command_interface_send_response(const char* response);

// Command handlers
void cmd_read_temperature(void);
void cmd_read_pressure(void);
void cmd_read_humidity(void);
void cmd_test_sensor(void);
void cmd_math_operation(char* command);

#endif // __COMMAND_INTERFACE_H__ 