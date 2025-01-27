/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define N_CHARS 91
#define N_CHARS2 39
#define N_BYTES 16
#define DATASET_LENGTH 10
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
DMA_HandleTypeDef hdma_i2c1_rx;
DMA_HandleTypeDef hdma_i2c2_rx;

RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart4;
DMA_HandleTypeDef hdma_uart4_tx;

/* USER CODE BEGIN PV */
uint16_t l1 = 0;
uint16_t l2 = 0;
uint16_t count = 0;
uint8_t flag_to_read_values = 0;
uint8_t flag_is_main_part = 0;
uint16_t dataset_count = 0;

RTC_TimeTypeDef sTime1 = {0};
RTC_TimeTypeDef sTime2 = {0};
RTC_DateTypeDef sDate1 = {0};
RTC_DateTypeDef sDate2 = {0};

uint8_t dat1[N_BYTES], dat2[N_BYTES], dat3[N_BYTES], dat4[N_BYTES];
uint8_t dat1_buf[N_BYTES], dat2_buf[N_BYTES], dat3_buf[N_BYTES], dat4_buf[N_BYTES];
uint8_t str[N_CHARS];// = "1234567812345678901234561234567890123456\n";
uint8_t str2[N_CHARS2];

uint32_t t1 = 0;
uint32_t t2 = 0;
uint16_t delta_t = 0;


uint8_t flag_t1_read = 0;
uint8_t flag_t2_read = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_UART4_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint32_t read_TIM2() {
  return TIM2->CNT;
}

int __io_putchar(int ch) {
  uint8_t c[1];
  c[0] = ch & 0x00FF;
  HAL_UART_Transmit(&huart4, &*c, 1, 10);
  return ch;
}

int _write(int file,char *ptr, int len) {
  int DataIdx;
  for(DataIdx= 0; DataIdx< len; DataIdx++) {
    __io_putchar(*ptr++);
  }
  return len;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	//HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8);
	//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
	//HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
	if (flag_is_main_part == 1) {
		if (GPIO_Pin == GPIO_PIN_3) {
			flag_to_read_values = 1;
			HAL_RTC_GetTime(&hrtc, &sTime1, RTC_FORMAT_BIN);
			HAL_RTC_GetDate(&hrtc, &sDate1, RTC_FORMAT_BIN);
			//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
		}
	}
	else {
		if (GPIO_Pin == GPIO_PIN_2) {
			t1 = read_TIM2();
			//flag_t1_read = 1;
		}
		else if (GPIO_Pin == GPIO_PIN_3) {
			t2 = read_TIM2();
			flag_t2_read = 1;

			dataset_count++;
			if (dataset_count >= DATASET_LENGTH) {
				flag_is_main_part = 1;
			}
		}
	}
	//HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
}

void setup_mpu(void) {
	uint8_t dat[] = {5, 16, 6, 1};
	uint8_t adds[] = {107, 55, 26, 56};
	uint8_t n_of_bytes = sizeof(dat) / sizeof(dat[0]);
	for (uint8_t idx=0; idx<n_of_bytes; idx++) {
		HAL_I2C_Mem_Write(&hi2c1, 0x68<<1, adds[idx], 1, &dat[idx], 1, 100);
		//delay(1000);
		HAL_I2C_Mem_Write(&hi2c2, 0x68<<1, adds[idx], 1, &dat[idx], 1, 100);
		//HAL_I2C_Mem_Write(&hi2c1, 0x69<<1, adds[idx], 1, &dat[idx], 1, 100);
		//HAL_I2C_Mem_Write(&hi2c2, 0x69<<1, adds[idx], 1, &dat[idx], 1, 100);
	}
	dat[0] = 7;
	adds[0] = 104;
	HAL_I2C_Mem_Write(&hi2c1, 0x68<<1, adds[0], 1, &dat[0], 1, 100);
	HAL_I2C_Mem_Write(&hi2c2, 0x68<<1, adds[0], 1, &dat[0], 1, 100);
	//HAL_I2C_Mem_Write(&hi2c1, 0x69<<1, adds[0], 1, &dat[0], 1, 100);
	//HAL_I2C_Mem_Write(&hi2c2, 0x69<<1, adds[0], 1, &dat[0], 1, 100);
}



void i2c_transaction(uint8_t sensor_num) {
	switch (sensor_num) {
		case 1 : HAL_I2C_Mem_Read_DMA(&hi2c1, 0x68<<1, 59, 1, dat1, 14); break;
		case 2 : HAL_I2C_Mem_Read_DMA(&hi2c2, 0x68<<1, 59, 1, dat2, 14); break;
		case 3 : HAL_I2C_Mem_Read_DMA(&hi2c1, 0x69<<1, 59, 1, dat3, 14); break;
		case 4 : HAL_I2C_Mem_Read_DMA(&hi2c2, 0x69<<1, 59, 1, dat4, 14); break;
		/*case 1 : HAL_I2C_Mem_Read(&hi2c1, 0x68<<1, 59, 1, dat1, 14, 10); break;
		case 2 : HAL_I2C_Mem_Read(&hi2c2, 0x68<<1, 59, 1, dat2, 14, 10); break;
		case 3 : HAL_I2C_Mem_Read(&hi2c1, 0x69<<1, 59, 1, dat3, 14, 10); break;
		case 4 : HAL_I2C_Mem_Read(&hi2c2, 0x69<<1, 59, 1, dat4, 14, 10); break;*/
		//HAL_I2C_Mem_Read(&hi2c1, 0x68<<1, 59, 1, dat1, 14, 10);
		//HAL_I2C_Mem_Read_DMA(&hi2c1, 0x68<<1, 59, 1, dat1, 14);
		//HAL_I2C_Mem_Read(&hi2c2, 0x68<<1, 59, 1, dat2, 14, 10);
		//HAL_I2C_Mem_Read_DMA(&hi2c2, 0x68<<1, 59, 1, dat2, 14);

	}
}

void make_string(void) {
	sprintf(str,
				"%02x %02x %04x " 													//11
				"%04x "																			//5
				"%04x %04x %04x %04x %04x %04x %04x " 			//35
				"%04x %04x %04x %04x %04x %04x %04x "			 	//35
				"%04x"                                      //4
				"\n", 																			//1
																										//=91
				(uint8_t)(sTime1.Minutes),
				(uint8_t)(sTime1.Seconds),
				(uint16_t)(sTime1.SubSeconds),

				count,

				(uint16_t)(dat1_buf[0]<<8 | dat1_buf[1]),
				(uint16_t)(dat1_buf[2]<<8 | dat1_buf[3]),
				(uint16_t)(dat1_buf[4]<<8 | dat1_buf[5]),
				(uint16_t)(dat1_buf[6]<<8 | dat1_buf[7]),
				(uint16_t)(dat1_buf[8]<<8 | dat1_buf[9]),
				(uint16_t)(dat1_buf[10]<<8 | dat1_buf[11]),
				(uint16_t)(dat1_buf[12]<<8 | dat1_buf[13]),

				(uint16_t)(dat2_buf[0]<<8 | dat2_buf[1]),
				(uint16_t)(dat2_buf[2]<<8 | dat2_buf[3]),
				(uint16_t)(dat2_buf[4]<<8 | dat2_buf[5]),
				(uint16_t)(dat2_buf[6]<<8 | dat2_buf[7]),
				(uint16_t)(dat2_buf[8]<<8 | dat2_buf[9]),
				(uint16_t)(dat2_buf[10]<<8 | dat2_buf[11]),
				(uint16_t)(dat2_buf[12]<<8 | dat2_buf[13]),

				delta_t
	);
}

void make_string2(void) {
	sprintf(str2,
		"%012d %012d %012d" //38
	  "\n", 			//1
								//=39
		t2-t1,
		t1,
		t2
	);
}

void delay(uint16_t n) {
	for (uint16_t i=0; i<n; i++) {
		;
	}
}

void cp() {
	for (uint8_t i=0; i<N_BYTES; i++) {
		dat1_buf[i] = dat1[i];
		dat2_buf[i] = dat2[i];
		//dat3_buf[i] = dat3[i];
		//dat4_buf[i] = dat4[i];
	}
}

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
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_UART4_Init();
  MX_RTC_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  HAL_NVIC_DisableIRQ(EXTI2_IRQn);
  HAL_NVIC_DisableIRQ(EXTI3_IRQn);
  HAL_TIM_Base_Start(&htim2);
  setup_mpu();
  //while(1) {
  //	;
  //}
  while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) != GPIO_PIN_SET) {
  	;
  }
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);
  while(flag_is_main_part == 0) {
  	if (flag_t2_read == 1) {
  		flag_t2_read = 0;
  	}
  }
  HAL_NVIC_DisableIRQ(EXTI2_IRQn);
  HAL_NVIC_DisableIRQ(EXTI3_IRQn);
  //make_string2();
	//HAL_UART_Transmit_DMA(&huart4, str2, N_CHARS2);
  delta_t = (uint16_t)(t2 - t1);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		if (flag_to_read_values == 1) {
			HAL_NVIC_DisableIRQ(EXTI3_IRQn);
			flag_to_read_values = 0;
			//HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
			count++;
			cp();
			i2c_transaction(1);
			i2c_transaction(2);
			make_string();
			//HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
			//delay(5000);//400//4000
			//i2c_transaction(4);
			//i2c_transaction(3);
			//delay(5000);
			HAL_UART_Transmit_DMA(&huart4, str, N_CHARS);
			//delay(5000);//18000
			if (count & 128) { HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);}//256>>2
			if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_2) != RESET) {
				__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_2);
			}
			HAL_NVIC_EnableIRQ(EXTI3_IRQn);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, sTime1.Seconds%2);
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
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_HSE_DIV25;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO2, RCC_MCO2SOURCE_SYSCLK, RCC_MCODIV_4);
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
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 400000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only 
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 32-1;
  hrtc.Init.SynchPrediv = 10000-1;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 3 - 1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967296 - 1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 2000000;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
  /* DMA1_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);
  /* DMA1_Stream4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA2 PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PD12 PD13 PD14 PD15 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PC9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
