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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* Standard library includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* FreeRTOS includes */
#include "timers.h"
#include "semphr.h"
#include "queue.h"

/* User includes */

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
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = { .name = "defaultTask",
		.stack_size = 128 * 4, .priority = (osPriority_t) osPriorityNormal, };
/* USER CODE BEGIN PV */

/* ----------------------------- VARIABLES ---------------------------- */

//static const TickType_t task_delay = 20 / portTICK_PERIOD_MS;
static volatile int isr_counter;
static volatile float adc_value;
static char avgCommand[] = "avg\n";

/* settings */
#define BUF_SIZE 10
#define MSG_LEN	50
#define QUEUE_LEN 5

/* double buffer */
static uint32_t buff_a[BUF_SIZE];
static uint32_t buff_b[BUF_SIZE];

/* buffer refs for swapping */
volatile static uint32_t *write_to = buff_a;
volatile static uint32_t *read_from = buff_b;

/* double buffer swap function */
static void swap_buffers() {
	volatile uint32_t *temp = write_to;
	write_to = read_from;
	read_from = temp;
}

/* flags */
static int buffer_overrun = 0;

/* Mutexes and Semaphores */
static SemaphoreHandle_t sem_read_complete;

/* Queues and related structs */
static xQueueHandle msg_queue;

typedef struct Message {
	char body[MSG_LEN];
} Message;

/* ------------------------------ TASKS ------------------------------ */

osThreadId_t calcTaskHandle;
const osThreadAttr_t calcTask_attributes = { .name = "calcTask", .stack_size =
		512 * 4, .priority = (osPriority_t) osPriorityNormal, };

osThreadId_t cliTaskHandle;
const osThreadAttr_t cliTask_attributes = { .name = "cliTask", .stack_size = 512
		* 4, .priority = (osPriority_t) osPriorityNormal, };


/* ----------------------------- CALLBACKS ---------------------------- */

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	static int idx = 0;
	BaseType_t task_can_be_awoken = pdFALSE;

	if (!buffer_overrun && idx < BUF_SIZE) {
		UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
		adc_value = HAL_ADC_GetValue(&hadc1);
		taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
		write_to[idx++] = adc_value;
	}

	if (idx >= BUF_SIZE) {
		if (xSemaphoreTakeFromISR(sem_read_complete, &task_can_be_awoken) == pdFALSE) {
			buffer_overrun = 1;
		}

		if (!buffer_overrun) {
//			UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
			swap_buffers();
//			taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
			idx = 0;

			/* Notify the calc task that the data is ready to be read */
			vTaskNotifyGiveFromISR((xTaskHandle)calcTaskHandle, &task_can_be_awoken);
		}
	}


	/* this is true when a context switch to a wake-able task is possible */
	if (task_can_be_awoken) {
		portYIELD_FROM_ISR(task_can_be_awoken);
	}

}



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM3_Init(void);
static void MX_ADC1_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */

/* ------------------------ FUNCTION SIGNATURES ----------------------- */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

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
	MX_TIM3_Init();
	MX_ADC1_Init();
	/* USER CODE BEGIN 2 */

	/* Start ADC */
	HAL_ADC_Start_IT(&hadc1);

	/* Create Mutexes and Semaphores */
	sem_read_complete = xSemaphoreCreateBinary();

	/* Disable internal buffering for stdin */
	setvbuf(stdin, NULL, _IONBF, 0);


	/* USER CODE END 2 */

	/* Init scheduler */
	osKernelInitialize();

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */

	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of defaultTask */
	defaultTaskHandle = osThreadNew(StartDefaultTask, NULL,
			&defaultTask_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */

	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
	/* USER CODE END RTOS_EVENTS */

	/* Start scheduler */
	osKernelStart();

	/* We should never get here as control is now taken by the scheduler */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void) {

	/* USER CODE BEGIN ADC1_Init 0 */

	/* USER CODE END ADC1_Init 0 */

	ADC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN ADC1_Init 1 */

	/* USER CODE END ADC1_Init 1 */

	/** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
	 */
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
	hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	hadc1.Init.ScanConvMode = DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
	hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T3_TRGO;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DMAContinuousRequests = DISABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	if (HAL_ADC_Init(&hadc1) != HAL_OK) {
		Error_Handler();
	}

	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	 */
	sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ADC1_Init 2 */

	/* USER CODE END ADC1_Init 2 */

}

/**
 * @brief TIM3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM3_Init(void) {

	/* USER CODE BEGIN TIM3_Init 0 */

	/* USER CODE END TIM3_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	/* USER CODE BEGIN TIM3_Init 1 */

	/* USER CODE END TIM3_Init 1 */
	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 1599;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 999;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim3) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM3_Init 2 */
	HAL_TIM_Base_Start_IT(&htim3);
	/* USER CODE END TIM3_Init 2 */

}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void) {

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
	if (HAL_UART_Init(&huart2) != HAL_OK) {
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
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */

	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

	/*Configure GPIO pin : PA5 */
	GPIO_InitStruct.Pin = GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* ----------------------- FUNCTION DEFINITIONS ----------------------- */

/* printf redirection */
int __io_putchar(int ch) {
	HAL_UART_Transmit(&huart2, (uint8_t*) &ch, 1, HAL_MAX_DELAY);
	return ch;
}

/* scanf redirection */
int __io_getchar(void) {
	uint8_t ch = 0;

	/* Clear the Overrun flag before reading the first character */
	/* Only clear overrun flag if overrun actually occurred */
	if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_ORE)) {
		__HAL_UART_CLEAR_OREFLAG(&huart2);
	}

	HAL_UART_Receive(&huart2, (uint8_t*) &ch, 1, HAL_MAX_DELAY);

	if (ch == '\r')
		ch = '\n';  // Convert CR to LF so gets() terminates

	HAL_UART_Transmit(&huart2, (uint8_t*) &ch, 1, HAL_MAX_DELAY);
	return ch;
}

/* ------------------- FUNCTION DEFINITIONS (TASKS) ------------------- */

void StartCalcTask(void *argument) {

	float avg;
	Message tx_msg;

	for (;;) {
		/* This will wait for the notification indefinitely (notified from ISR) */
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		avg = 0.0f;
		for (int i = 0; i < BUF_SIZE; i++) {
			avg += read_from[i];
		}
		avg /= (float)BUF_SIZE;

		taskENTER_CRITICAL();
		adc_value = avg;
		taskEXIT_CRITICAL();

		if (buffer_overrun) {
			strcpy(tx_msg.body, "[Error] Buffer overrun, samples dropped");
			xQueueSend(msg_queue, (void *)&tx_msg, 10);
		}

		taskENTER_CRITICAL();
		/* Reset the flag, and give the "done reading" semaphore - atomic */
		buffer_overrun = 0;
		xSemaphoreGive(sem_read_complete);
		taskEXIT_CRITICAL();

	}
}

void StartCliTask(void *argument) {
	Message rx_msg;
	char input_line[25];
	for (;;) {

		if (xQueueReceive(msg_queue, (void *)&rx_msg, 0) == pdTRUE) {
			printf("%s\r\n", rx_msg.body);
		}

		fgets(input_line, 25, stdin);

		if (strcmp(input_line, avgCommand) == 0) {
			printf("Average: %g\r\n", adc_value);
		}

//		vTaskDelay(task_delay);
	}
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument) {
	/* USER CODE BEGIN 5 */

	/* safeguards */
	if (sem_read_complete == NULL) {
		printf("[ERROR] A Semaphore could not be created!\r\n");
		vTaskDelete(NULL);
	}

	/* Initially, we need to set the binary semaphore to 1, so that a read can happen */
	xSemaphoreGive(sem_read_complete);

	/* Initialize values */
	msg_queue = xQueueCreate(QUEUE_LEN, sizeof(Message));

	calcTaskHandle = osThreadNew(StartCalcTask, NULL, &calcTask_attributes);
	cliTaskHandle = osThreadNew(StartCliTask, NULL, &cliTask_attributes);

	vTaskDelete(NULL);
	/* USER CODE END 5 */
}

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM11 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	/* USER CODE BEGIN Callback 0 */
//	if (htim->Instance == TIM3) {
//
//	}
	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM11) {
		HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */

	/* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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
