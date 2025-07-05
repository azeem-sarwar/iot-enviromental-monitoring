/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bme680_interface.h"
#include "command_interface.h"
#include "lora_interface.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart4;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART4_UART_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_USART4_UART_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  // System initialization messages
  command_interface_send_response("========================================\r\n");
  command_interface_send_response("IoT Prototype System - STM32G071RB\r\n");
  command_interface_send_response("========================================\r\n");
  command_interface_send_response("System Clock: 16 MHz\r\n");
  command_interface_send_response("I2C1 Configuration: PA9 (SCL), PA10 (SDA)\r\n");
  command_interface_send_response("USART2: PA2 (TX), PA3 (RX) - 115200 baud\r\n");
  command_interface_send_response("USART4: PA0 (TX), PA1 (RX) - 115200 baud\r\n");
  command_interface_send_response("SPI1: PA5 (SCK), PA6 (MISO), PA7 (MOSI)\r\n");
  command_interface_send_response("LoRa: PA4 (NSS), PC0 (RESET)\r\n");
  command_interface_send_response("LED Status: PA5\r\n");
  command_interface_send_response("========================================\r\n");
  
  command_interface_send_response_usart4("========================================\r\n");
  command_interface_send_response_usart4("IoT Prototype System - STM32G071RB\r\n");
  command_interface_send_response_usart4("========================================\r\n");
  command_interface_send_response_usart4("System Clock: 16 MHz\r\n");
  command_interface_send_response_usart4("I2C1 Configuration: PA9 (SCL), PA10 (SDA)\r\n");
  command_interface_send_response_usart4("USART2: PA2 (TX), PA3 (RX) - 115200 baud\r\n");
  command_interface_send_response_usart4("USART4: PA0 (TX), PA1 (RX) - 115200 baud\r\n");
  command_interface_send_response_usart4("SPI1: PA5 (SCK), PA6 (MISO), PA7 (MOSI)\r\n");
  command_interface_send_response_usart4("LoRa: PA4 (NSS), PC0 (RESET)\r\n");
  command_interface_send_response_usart4("LED Status: PA5\r\n");
  command_interface_send_response_usart4("========================================\r\n");
  
  // Scan I2C bus for devices
  command_interface_send_response("\r\nScanning I2C bus for devices...\r\n");
  command_interface_send_response_usart4("\r\nScanning I2C bus for devices...\r\n");
  i2c_scan_bus();
  
  // Test I2C configuration
  command_interface_send_response("\r\nTesting I2C configuration...\r\n");
  command_interface_send_response_usart4("\r\nTesting I2C configuration...\r\n");
  
  // Test I2C bus with a simple ping
  HAL_StatusTypeDef i2c_test = HAL_I2C_IsDeviceReady(&hi2c1, 0x76 << 1, 3, 1000);
  if (i2c_test == HAL_OK) {
    command_interface_send_response("✓ I2C bus test successful - device responds at 0x76\r\n");
    command_interface_send_response_usart4("✓ I2C bus test successful - device responds at 0x76\r\n");
  } else {
    command_interface_send_response("✗ I2C bus test failed - no device at 0x76\r\n");
    command_interface_send_response_usart4("✗ I2C bus test failed - no device at 0x76\r\n");
  }
  
  // Check BME680 sensor presence
  command_interface_send_response("\r\nChecking BME680 sensor presence...\r\n");
  command_interface_send_response_usart4("\r\nChecking BME680 sensor presence...\r\n");
  
  if (bme680_check_sensor_presence() == BME68X_OK) {
    command_interface_send_response("✓ BME680 sensor detected on I2C bus (Address: 0x76)\r\n");
    command_interface_send_response_usart4("✓ BME680 sensor detected on I2C bus (Address: 0x76)\r\n");
    
    // Initialize BME680 sensor
    command_interface_send_response("Initializing BME680 sensor...\r\n");
    command_interface_send_response_usart4("Initializing BME680 sensor...\r\n");
    
    if (bme680_init_sensor() == BME68X_OK) {
      command_interface_send_response("✓ BME680 sensor initialized successfully\r\n");
      command_interface_send_response("  - Temperature oversampling: 1x\r\n");
      command_interface_send_response("  - Pressure oversampling: 1x\r\n");
      command_interface_send_response("  - Humidity oversampling: 1x\r\n");
      command_interface_send_response("  - Gas sensor: Disabled\r\n");
      
      command_interface_send_response_usart4("✓ BME680 sensor initialized successfully\r\n");
      command_interface_send_response_usart4("  - Temperature oversampling: 1x\r\n");
      command_interface_send_response_usart4("  - Pressure oversampling: 1x\r\n");
      command_interface_send_response_usart4("  - Humidity oversampling: 1x\r\n");
      command_interface_send_response_usart4("  - Gas sensor: Disabled\r\n");
    } else {
      command_interface_send_response("✗ Error initializing BME680 sensor\r\n");
      command_interface_send_response("  - Check sensor power supply (3.3V)\r\n");
      command_interface_send_response("  - Verify I2C connections\r\n");
      
      command_interface_send_response_usart4("✗ Error initializing BME680 sensor\r\n");
      command_interface_send_response_usart4("  - Check sensor power supply (3.3V)\r\n");
      command_interface_send_response_usart4("  - Verify I2C connections\r\n");
    }
  } else {
    command_interface_send_response("✗ BME680 sensor not found on I2C bus\r\n");
    command_interface_send_response("Troubleshooting steps:\r\n");
    command_interface_send_response("  1. Check I2C connections:\r\n");
    command_interface_send_response("     - PA9 (SCL) → BME680 SCL\r\n");
    command_interface_send_response("     - PA10 (SDA) → BME680 SDA\r\n");
    command_interface_send_response("  2. Verify power supply:\r\n");
    command_interface_send_response("     - BME680 VCC → 3.3V\r\n");
    command_interface_send_response("     - BME680 GND → GND\r\n");
    command_interface_send_response("  3. Check pull-up resistors (4.7kΩ recommended)\r\n");
    command_interface_send_response("  4. Verify I2C address (default: 0x76)\r\n");
    command_interface_send_response("System will continue without sensor functionality\r\n");
    
    command_interface_send_response_usart4("✗ BME680 sensor not found on I2C bus\r\n");
    command_interface_send_response_usart4("Troubleshooting steps:\r\n");
    command_interface_send_response_usart4("  1. Check I2C connections:\r\n");
    command_interface_send_response_usart4("     - PA9 (SCL) → BME680 SCL\r\n");
    command_interface_send_response_usart4("     - PA10 (SDA) → BME680 SDA\r\n");
    command_interface_send_response_usart4("  2. Verify power supply:\r\n");
    command_interface_send_response_usart4("     - BME680 VCC → 3.3V\r\n");
    command_interface_send_response_usart4("     - BME680 GND → GND\r\n");
    command_interface_send_response_usart4("  3. Check pull-up resistors (4.7kΩ recommended)\r\n");
    command_interface_send_response_usart4("  4. Verify I2C address (default: 0x76)\r\n");
    command_interface_send_response_usart4("System will continue without sensor functionality\r\n");
  }
  
  // Initialize LoRa module
  command_interface_send_response("\r\nInitializing LoRa module...\r\n");
  command_interface_send_response_usart4("\r\nInitializing LoRa module...\r\n");
  
  if (lora_init() == 0) {
    command_interface_send_response("✓ LoRa module initialized successfully\r\n");
    command_interface_send_response_usart4("✓ LoRa module initialized successfully\r\n");
  } else {
    command_interface_send_response("✗ LoRa module initialization failed\r\n");
    command_interface_send_response_usart4("✗ LoRa module initialization failed\r\n");
  }
  
  // Initialize command interface
  command_interface_init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // Process command interface
    command_interface_process();
    
    // Toggle LED to show system is running
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    
    // Small delay
    HAL_Delay(10);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00503D58;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART4_UART_Init(void)
{

  /* USER CODE BEGIN USART4_Init 0 */

  /* USER CODE END USART4_Init 0 */

  /* USER CODE BEGIN USART4_Init 1 */

  /* USER CODE END USART4_Init 1 */
  huart4.Instance = USART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART4_Init 2 */

  /* USER CODE END USART4_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET); // LoRa NSS high initially

  /*Configure GPIO pins : PC0 PC1 PC2 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  
  /*Configure LoRa NSS pin (PA4) */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  /*Configure SPI1 pins: PA5 (SCK), PA6 (MISO), PA7 (MOSI) */
  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
