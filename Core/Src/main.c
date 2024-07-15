/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#include "dma.h"
#include "usart.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LIN_SEND_PERIOD 100
#define RX_TX_BUFFER_SIZE 12 // if you can make it work for >12, please share your solution with me :)
#define FAN_POWER_MAX 7
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void Process_Received_LIN_Data(void);

// https://controllerstech.com/stm32-uart-8-lin-protocol-part-1/
// https://controllerstech.com/stm32-uart-9-lin-protocol-part-2/
// https://controllerstech.com/stm32-uart-10-lin-protocol-part-3/
uint8_t Pid_Calc(uint8_t ID);
uint8_t Checksum_Calc(uint8_t PID, uint8_t *data, uint8_t size);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t LeftSideTemp = 0xD;
uint8_t RightSideTemp = 0xD;
uint8_t RearDefrost;
uint8_t FanPower = 0;
uint32_t FanPwmDuty = 1000; //out of 1000
uint8_t TxData[RX_TX_BUFFER_SIZE];
uint8_t RxData[RX_TX_BUFFER_SIZE];
uint8_t RxDataPrevious[RX_TX_BUFFER_SIZE];
volatile uint8_t lin_data_received_flag = 0;
extern DMA_HandleTypeDef hdma_usart3_rx;

uint32_t LinSendSoftTimer;

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
	MX_LPUART1_UART_Init();
	MX_USART3_UART_Init();
	MX_TIM15_Init();
	/* USER CODE BEGIN 2 */

	LinSendSoftTimer = HAL_GetTick();

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		if (lin_data_received_flag == 1)
		{
			lin_data_received_flag = 0;
			Process_Received_LIN_Data();
		}

		if (HAL_GetTick() - LinSendSoftTimer > LIN_SEND_PERIOD)
		{
			LinSendSoftTimer = HAL_GetTick();

			// send status
			TxData[0] = 0x55;  // sync field
			TxData[1] = 0xB1;  // Pid_Calc(0x31);

			TxData[2] = 0x80;
			TxData[3] = (0x20 | (FanPower & 0x0F));
			TxData[4] = 0x14;
			TxData[5] = (0x40 & (0xFF * RearDefrost));
			TxData[6] = LeftSideTemp;
			TxData[7] = RightSideTemp;
			TxData[8] = 0x00;
			TxData[9] = 0xc1;  // data byte 7

			TxData[10] = Checksum_Calc(TxData[1], TxData + 2, 8); // lin 2.1 includes PID

			HAL_LIN_SendBreak(&huart3);
			HAL_UART_Transmit(&huart3, TxData, 11, 100);

			FanPwmDuty = 800 * FanPower / FAN_POWER_MAX
					+ (200 * (FanPower != 0));
			HAL_TIM_PWM_Start_DMA(&htim15, TIM_CHANNEL_1, &FanPwmDuty, 1);

			HAL_Delay(1);

			// ask for buttons and knobs
			TxData[0] = 0x55;  // sync field
			TxData[1] = 0x39;  // Pid_Calc(0x39);

			TxData[2] = Checksum_Calc(TxData[1], TxData + 2, 0); // lin 2.1 includes PID

			memset(RxData, 0x00, RX_TX_BUFFER_SIZE);
			HAL_UARTEx_ReceiveToIdle_DMA(&huart3, RxData, RX_TX_BUFFER_SIZE);
			__HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);

			HAL_LIN_SendBreak(&huart3);
			HAL_UART_Transmit_DMA(&huart3, TxData, 2);

			HAL_GPIO_WritePin(LOGIC_ANALYZER_1_GPIO_Port, LOGIC_ANALYZER_1_Pin,
					GPIO_PIN_RESET);
			HAL_GPIO_WritePin(LOGIC_ANALYZER_0_GPIO_Port, LOGIC_ANALYZER_0_Pin,
					GPIO_PIN_RESET);

			HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
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
	RCC_OscInitTypeDef RCC_OscInitStruct =
	{ 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct =
	{ 0 };

	/** Configure the main internal regulator output voltage
	 */
	HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
	RCC_OscInitStruct.PLL.PLLN = 85;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
	{
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART3)
	{
		;
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART3)
	{
		lin_data_received_flag = 1;
		HAL_GPIO_WritePin(LOGIC_ANALYZER_1_GPIO_Port, LOGIC_ANALYZER_1_Pin,
				GPIO_PIN_SET);
	}
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) // never called :(
{
	if (huart->Instance == USART3)
	{
		lin_data_received_flag = 1;
		HAL_GPIO_WritePin(LOGIC_ANALYZER_0_GPIO_Port, LOGIC_ANALYZER_0_Pin,
				GPIO_PIN_SET);
	}
}

void Process_Received_LIN_Data(void)
{
	//right knob
	if (RxData[8] == 0x91)
	{
		RightSideTemp++;
	}
	else if (RxData[8] == 0x92)
	{
		RightSideTemp += 2;

	}
	else if (RxData[8] == 0x93)
	{
		RightSideTemp += 3;
	}

	if (RxData[8] == 0x8F)
	{
		RightSideTemp--;
	}
	else if (RxData[8] == 0x8E)
	{
		RightSideTemp -= 2;

	}
	else if (RxData[8] == 0x8D)
	{
		RightSideTemp -= 3;
	}

	//left knob
	if (RxData[7] == 0x11)
	{
		LeftSideTemp++;
	}
	else if (RxData[7] == 0x12)
	{
		LeftSideTemp += 2;

	}
	else if (RxData[7] == 0x13)
	{
		LeftSideTemp += 3;
	}

	if (RxData[7] == 0x0F)
	{
		LeftSideTemp--;
	}
	else if (RxData[7] == 0x0E)
	{
		LeftSideTemp -= 2;

	}
	else if (RxData[7] == 0x0D)
	{
		LeftSideTemp -= 3;
	}

	// front defrost
	if ((RxData[6] == 0x40) && (RxDataPrevious[6] == 0x00))
	{

		RearDefrost ^= 1;
	}

	// fun speed
	if ((RxData[4] == 0x3D) && (RxDataPrevious[4] == 0x00) && (FanPower > 0))
	{

		FanPower--;
	}
	else if ((RxData[4] == 0x3C) && (RxDataPrevious[4] == 0x00)
			&& (FanPower < FAN_POWER_MAX))
	{
		FanPower++;
	}

	memcpy(RxDataPrevious, RxData, RX_TX_BUFFER_SIZE);
}

uint8_t Pid_Calc(uint8_t ID)
{
	if (ID > 0x3F)
		Error_Handler();
	uint8_t IDBuf[6];
	for (int i = 0; i < 6; i++)
	{
		IDBuf[i] = (ID >> i) & 0x01;
	}

	uint8_t P0 = (IDBuf[0] ^ IDBuf[1] ^ IDBuf[2] ^ IDBuf[4]) & 0x01;
	uint8_t P1 = ~((IDBuf[1] ^ IDBuf[3] ^ IDBuf[4] ^ IDBuf[5]) & 0x01);

	ID = ID | (P0 << 6) | (P1 << 7);
	return ID;
}

uint8_t Checksum_Calc(uint8_t PID, uint8_t *data, uint8_t size)
{
	uint8_t buffer[size + 2];
	uint16_t sum = 0;
	buffer[0] = PID;
	for (int i = 0; i < size; i++)
	{
		buffer[i + 1] = data[i];
	}

	for (int i = 0; i < size + 1; i++)
	{
		sum = sum + buffer[i];
		if (sum > 0xff)
			sum = sum - 0xff;
	}

	sum = 0xff - sum;
	return sum;
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
