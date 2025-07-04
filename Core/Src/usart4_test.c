#include "usart4_test.h"
#include "stm32g0xx_hal.h"
#include <string.h>

extern UART_HandleTypeDef huart4;  // Provided by MX_USART4_UART_Init()

void USART4_SendString(const char* str) {
    HAL_UART_Transmit(&huart4, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}

void USART4_Test(void) {
    char rx_buffer[32] = {0};

    USART4_SendString("USART4 Test OK\r\n");
    USART4_SendString("Type something: ");

    if (HAL_UART_Receive(&huart4, (uint8_t*)rx_buffer, sizeof(rx_buffer) - 1, 5000) == HAL_OK) {
        USART4_SendString("\r\nYou typed: ");
        USART4_SendString(rx_buffer);
        USART4_SendString("\r\n");
    } else {
        USART4_SendString("\r\n[No input received within 5 seconds]\r\n");
    }
}
