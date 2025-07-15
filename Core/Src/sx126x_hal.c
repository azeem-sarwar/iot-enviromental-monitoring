/**
 * @file      sx126x_hal.c
 *
 * @brief     Hardware Abstraction Layer for SX126x
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Semtech corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include "sx126x_hal.h"
#include "stm32g0xx_hal.h"
#include <string.h>

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

typedef struct sx126x_hal_context_s
{
    SPI_HandleTypeDef* spi;
    GPIO_TypeDef* nss_port;
    uint16_t nss_pin;
    GPIO_TypeDef* reset_port;
    uint16_t reset_pin;
} sx126x_hal_context_t;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

// External handles
extern SPI_HandleTypeDef hspi1;

// HAL context
static sx126x_hal_context_t hal_ctx = {
    .spi = &hspi1,
    .nss_port = GPIOA,
    .nss_pin = GPIO_PIN_4,
    .reset_port = GPIOC,
    .reset_pin = GPIO_PIN_0
};

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

static void sx126x_hal_spi_select(void);
static void sx126x_hal_spi_deselect(void);

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

sx126x_hal_status_t sx126x_hal_write(const void* context, const uint8_t* command, const uint16_t command_length,
                                     const uint8_t* data, const uint16_t data_length)
{
    (void)context; // Unused parameter
    
    sx126x_hal_spi_select();
    
    // Send command with longer timeout for better reliability
    HAL_StatusTypeDef status = HAL_SPI_Transmit(hal_ctx.spi, (uint8_t*)command, command_length, 500);
    
    // Send data if present
    if ((status == HAL_OK) && (data != NULL) && (data_length > 0)) {
        status = HAL_SPI_Transmit(hal_ctx.spi, (uint8_t*)data, data_length, 500);
    }
    
    sx126x_hal_spi_deselect();
    
    // Additional check: if no device is connected, SPI might still return HAL_OK
    // but the actual communication failed. We can't easily detect this at HAL level.
    return (status == HAL_OK) ? SX126X_HAL_STATUS_OK : SX126X_HAL_STATUS_ERROR;
}

sx126x_hal_status_t sx126x_hal_read(const void* context, const uint8_t* command, const uint16_t command_length,
                                    uint8_t* data, const uint16_t data_length)
{
    (void)context; // Unused parameter
    
    sx126x_hal_spi_select();
    
    // Send command with longer timeout for better reliability
    HAL_StatusTypeDef status = HAL_SPI_Transmit(hal_ctx.spi, (uint8_t*)command, command_length, 500);
    
    // Receive data if present
    if ((status == HAL_OK) && (data != NULL) && (data_length > 0)) {
        status = HAL_SPI_Receive(hal_ctx.spi, data, data_length, 500);
    }
    
    sx126x_hal_spi_deselect();
    
    return (status == HAL_OK) ? SX126X_HAL_STATUS_OK : SX126X_HAL_STATUS_ERROR;
}

sx126x_hal_status_t sx126x_hal_reset(const void* context)
{
    (void)context; // Unused parameter
    
    // Reset the radio
    HAL_GPIO_WritePin(hal_ctx.reset_port, hal_ctx.reset_pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(hal_ctx.reset_port, hal_ctx.reset_pin, GPIO_PIN_SET);
    HAL_Delay(100); // Wait for module to boot
    
    return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_hal_wakeup(const void* context)
{
    (void)context; // Unused parameter
    
    // For SX126x, wakeup is typically done by pulling NSS low briefly
    // or by sending a specific command. For now, we'll just return OK.
    // The actual wakeup will be handled by the SX126x driver functions.
    
    return SX126X_HAL_STATUS_OK;
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static void sx126x_hal_spi_select(void)
{
    HAL_GPIO_WritePin(hal_ctx.nss_port, hal_ctx.nss_pin, GPIO_PIN_RESET);
}

static void sx126x_hal_spi_deselect(void)
{
    HAL_GPIO_WritePin(hal_ctx.nss_port, hal_ctx.nss_pin, GPIO_PIN_SET);
}

/* --- EOF ------------------------------------------------------------------ */ 