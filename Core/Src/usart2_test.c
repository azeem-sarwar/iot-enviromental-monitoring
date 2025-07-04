#include "usart2_test.h"
#include "stm32g0xx_hal.h"
#include <string.h>

extern UART_HandleTypeDef huart2;  // Use already-initialized USART2

void USART2_SendString(const char* str) {
    HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}

void USART2_Test(void) {
    char rx_buffer[32] = {0};

    USART2_SendString("USART2 Test OK\r\n");
    USART2_SendString("Type something: ");

    if (HAL_UART_Receive(&huart2, (uint8_t*)rx_buffer, sizeof(rx_buffer) - 1, 5000) == HAL_OK) {
        USART2_SendString("\r\nYou typed: ");
        USART2_SendString(rx_buffer);
        USART2_SendString("\r\n");
    } else {
        USART2_SendString("\r\n[No input received within 5 seconds]\r\n");
    }
}
