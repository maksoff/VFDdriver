/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "freertos_inc.h"
#include "microrl_cmd.h"
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
/* USER CODE BEGIN Variables */


/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for LEDheartbeat */
osThreadId_t LEDheartbeatHandle;
const osThreadAttr_t LEDheartbeat_attributes = {
  .name = "LEDheartbeat",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for taskUSB_rcv */
osThreadId_t taskUSB_rcvHandle;
const osThreadAttr_t taskUSB_rcv_attributes = {
  .name = "taskUSB_rcv",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for qUSB_rcv */
osMessageQueueId_t qUSB_rcvHandle;
const osMessageQueueAttr_t qUSB_rcv_attributes = {
  .name = "qUSB_rcv"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */


/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartLEDheartbeat(void *argument);
void StartUSB_rcv(void *argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of qUSB_rcv */
  qUSB_rcvHandle = osMessageQueueNew (64, sizeof(uint8_t), &qUSB_rcv_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of LEDheartbeat */
  LEDheartbeatHandle = osThreadNew(StartLEDheartbeat, NULL, &LEDheartbeat_attributes);

  /* creation of taskUSB_rcv */
  taskUSB_rcvHandle = osThreadNew(StartUSB_rcv, NULL, &taskUSB_rcv_attributes);

  /* USER CODE BEGIN RTOS_THREADS */

  qUSB_rcvQueue = qUSB_rcvHandle; // adding to freertos_inc.h
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDefaultTask */


  // Enable USB pull-up
  HAL_GPIO_WritePin(USB_PU_GPIO_Port, USB_PU_Pin, GPIO_PIN_SET);
  osDelay(10);
  init_microrl();
  set_CDC_ready(); // allow to send

  vTaskDelete(NULL);
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartLEDheartbeat */
/**
* @brief Function implementing the LEDheartbeat thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLEDheartbeat */
void StartLEDheartbeat(void *argument)
{
  /* USER CODE BEGIN StartLEDheartbeat */
	TickType_t xLastWakeTime;
	const TickType_t xPeriod = 500 / portTICK_PERIOD_MS;
	/* Infinite loop */
	for (;;) {
		xLastWakeTime = xTaskGetTickCount();
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		vTaskDelayUntil(&xLastWakeTime, xPeriod);
	}
  /* USER CODE END StartLEDheartbeat */
}

void SEGGER_u32(uint32_t dig)
{
	char str [8];
	for (int i = 0; i < 5; i++)
	{
		str[4 - i] = dig % 10 + '0';
		dig /= 10;
	}
	str[5] = '\r';
	str[6] = '\n';
	str[7] = '\0';
	SEGGER_RTT_WriteString(0, str);
}

/* USER CODE BEGIN Header_StartUSB_rcv */
/**
* @brief Function implementing the taskUSB_rcv thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUSB_rcv */
void StartUSB_rcv(void *argument)
{
  /* USER CODE BEGIN StartUSB_rcv */
  /* Infinite loop */
	char buf;

	UBaseType_t uxHighWaterMark;

	/* Inspect our own high water mark on entering the task. */
	uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
	SEGGER_u32((uint32_t)uxHighWaterMark);

  for(;;)
  {
	  xQueueReceive(qUSB_rcvQueue, &buf, portMAX_DELAY );
	  microrl_print_char(buf);
	  uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
	  SEGGER_u32((uint32_t)uxHighWaterMark);

  }
  /* USER CODE END StartUSB_rcv */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
