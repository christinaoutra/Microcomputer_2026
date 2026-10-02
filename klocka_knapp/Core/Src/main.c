/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "quad_sseg.h"
#include "lcd.h"
#include <stdio.h> //Needed for printf() and scanf()
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// Debounce time measured with the oscilloscope.
#define BOUNCE_DELAY_MS 4

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

// LCD-skärmen
TextLCDType lcd;

// Counts all button interrupts, including bouncing.
uint16_t button_exti_count;

// Counts valid button presses after debouncing.
uint16_t button_debounced_count;

// Stores the time of the latest button interrupt.
uint32_t last_flank_causing_exti;

// Stores the last interrupt that has already been handled.
uint32_t last_managed_exti;

// Clock time.
volatile uint8_t clock_hours = 0;
volatile uint8_t clock_minutes = 0;
volatile uint8_t clock_seconds = 0;

// Counts the half-second timer events.
volatile uint8_t clock_half_second = 0;

// Controls if the colon is on or off.
volatile uint8_t clock_colon = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM6_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

// Print the UART menu.
void uart_print_menu(void);

// Read one menu choice from UART.
int uart_get_menu_choice(void);

// Print message for invalid choice.
void uart_print_bad_choice(void);

// Start clock mode.
void clock_mode(void);

// Start button mode.
void button_mode(void);

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
  MX_TIM6_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

  // Testar om LCD-skärmen svarar via I2C
  HAL_StatusTypeDef i2c_test;
  i2c_test = HAL_I2C_IsDeviceReady(&hi2c1, 0x4E, 3, 100);

  // Startar LCD-skärmen
  TextLCD_Init(&lcd, &hi2c1, 0x4E);


//test till display
/*
  for (int i = 0; i < 10; i++)
  {
      uint32_t dly = 250;

      qs_put_big_num(i);
      HAL_Delay(dly);

      qs_put_digits(i, i, i, i, 0);
      HAL_Delay(dly);

      qs_put_digits(i, i, i, i, 1);
      HAL_Delay(dly);
  }
*/
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
		// Print the menu.
		uart_print_menu();

		// Read the user's choice.
		int menu_choice = uart_get_menu_choice();

		// Choose what to do.
		switch (menu_choice) {
		case 1:
			clock_mode();
			break;

		case 2:
			button_mode();
			break;

		default:
			uart_print_bad_choice();
			break;
		}

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
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
  hi2c1.Init.Timing = 0x10D19CE4;
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
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 3999;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 9999;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

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
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, SEG_DIO_Pin|SEG_CLK_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, SMPS_EN_Pin|SMPS_V1_Pin|SMPS_SW_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : B1_Pin MY_BTN_Pin */
  GPIO_InitStruct.Pin = B1_Pin|MY_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : SEG_DIO_Pin */
  GPIO_InitStruct.Pin = SEG_DIO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SEG_DIO_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SEG_CLK_Pin */
  GPIO_InitStruct.Pin = SEG_CLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SEG_CLK_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SMPS_EN_Pin SMPS_V1_Pin SMPS_SW_Pin */
  GPIO_InitStruct.Pin = SMPS_EN_Pin|SMPS_V1_Pin|SMPS_SW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : SMPS_PG_Pin */
  GPIO_InitStruct.Pin = SMPS_PG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SMPS_PG_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD4_Pin */
  GPIO_InitStruct.Pin = LD4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD4_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

// Send printf() text through UART2.
int _write(int file, char *ptr, int len) {
	HAL_UART_Transmit(&huart2, (uint8_t*) ptr, len,
	HAL_MAX_DELAY);

	return len;
}

// Runs when a GPIO interrupt happens.
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    // Check that the interrupt came from our external button.
    if (GPIO_Pin == MY_BTN_Pin)
    {
        // Count every interrupt, including contact bounce.
        button_exti_count++;

        // Save the time when the latest interrupt happened.
        last_flank_causing_exti = HAL_GetTick();
    }
}

void uart_print_menu ()
{
	printf("\r\nMenu:\r\n");
	printf("1. Clock mode\r\n");
	printf("2. Button mode\r\n");
	printf("Choose: \r\n");
}

int uart_get_menu_choice()
{
    char str[1] = { '\0' };
    uint16_t str_len = 1;

    HAL_UART_Receive(&huart2,
                     (uint8_t *)str,
                     str_len,
                     HAL_MAX_DELAY);

    int ret = -1;
    sscanf(str, "%d", &ret);

    return ret;
}

// Runs every time a timer interrupt happens.
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    // Check that the interrupt came from TIM6.
    if (htim->Instance == TIM6)
    {
    	// Toggle colon every 0.5 seconds.
    	clock_colon = !clock_colon;

    	// Count half-second events.
    	clock_half_second++;

    	// Two half-second events make one second.
    	if (clock_half_second >= 2)
    	{
    	    clock_half_second = 0;
    	    clock_seconds++;

    	    // 60 seconds make one minute.
    	    if (clock_seconds >= 60)
    	    {
    	        clock_seconds = 0;
    	        clock_minutes++;

    	        // 60 minutes make one hour.
    	        if (clock_minutes >= 60)
    	        {
    	            clock_minutes = 0;
    	            clock_hours++;

    	            // Start a new day after 24 hours.
    	            if (clock_hours >= 24)
    	            {
    	                clock_hours = 0;
    	            }
    	        }
    	    }
    	}
    }
}

void uart_print_bad_choice()
{
    // Tell the user that the menu choice was invalid.
    printf("\r\nInvalid choice.\r\n");
}

void clock_mode()
{
	// Show that clock mode has started.
	printf("\r\nClock mode started\r\n");

	clock_hours = 23;
	clock_minutes = 59;
	clock_seconds = 45;


	// Start TIM6 with interrupt.
	HAL_TIM_Base_Start_IT(&htim6);

	char lcd_time[9];

    while (1)
    {

    	// Visar tiden på LCD-skärmen
    	sprintf(lcd_time, "%02d:%02d:%02d",
    	        clock_hours,
    	        clock_minutes,
    	        clock_seconds);

    	TextLCD_Position(&lcd, 8, 1);
    	TextLCD_PutStr(&lcd, lcd_time);

    	int b1_pressed = GPIO_PIN_SET
    	        == HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);

    	if (b1_pressed)
    	{
    	    // Show HH:MM when B1 is pressed.
    	    qs_put_digits(
    	        clock_hours / 10,
    	        clock_hours % 10,
    	        clock_minutes / 10,
    	        clock_minutes % 10,
    	        clock_colon
    	    );
    	}
    	else
    	{
    		// Show MM:SS on the display.
    		qs_put_digits(
    			clock_minutes / 10,
				clock_minutes % 10,
				clock_seconds / 10,
				clock_seconds % 10,
				clock_colon
    		);
    	}
    }
}

void button_mode()
{
    // Store current time.
    uint32_t now;

    // Show that button mode has started.
    printf("\r\nButton mode started\r\n");

    while (1)
    {
        // Read current system time in milliseconds.
        now = HAL_GetTick();

        // Store if the blue B1 button is pressed.
        int b1_pressed;

        // Check if a new interrupt has happened.
        if (last_flank_causing_exti != last_managed_exti)
        {
            // Check if enough time has passed since the latest flank.
            if ((now - last_flank_causing_exti) >= BOUNCE_DELAY_MS)
            {
                // Mark this interrupt as handled.
                last_managed_exti = last_flank_causing_exti;

				// Check if the button is still pressed.
				if (HAL_GPIO_ReadPin(MY_BTN_GPIO_Port, MY_BTN_Pin)
						== GPIO_PIN_RESET) {
					// Count one valid button press.
					button_debounced_count++;

				}
			}
        }

		// Check the blue B1 button on the NUCLEO board.
		b1_pressed = GPIO_PIN_RESET
				== HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);

		// Show raw interrupts when B1 is held.
		// Otherwise show debounced button presses.
		qs_put_big_num(
				b1_pressed ?
						button_exti_count : button_debounced_count);
    }
}

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
#ifdef USE_FULL_ASSERT
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
