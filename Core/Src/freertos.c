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
#include "usart.h"
#include "spi.h"
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
uint16_t encoder_value = 0;

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
/* Definitions for UARTtask */
osThreadId_t UARTtaskHandle;
const osThreadAttr_t UARTtask_attributes = {
  .name = "UARTtask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Encoder */
osThreadId_t EncoderHandle;
const osThreadAttr_t Encoder_attributes = {
  .name = "Encoder",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for qUSB_rcv */
osMessageQueueId_t qUSB_rcvHandle;
const osMessageQueueAttr_t qUSB_rcv_attributes = {
  .name = "qUSB_rcv"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void process_encoder(void);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartLEDheartbeat(void *argument);
void StartUSB_rcv(void *argument);
void StartUARTtask(void *argument);
void StartEncoder(void *argument);

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

  /* creation of UARTtask */
  UARTtaskHandle = osThreadNew(StartUARTtask, NULL, &UARTtask_attributes);

  /* creation of Encoder */
  EncoderHandle = osThreadNew(StartEncoder, NULL, &Encoder_attributes);

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

  //vTaskDelete(NULL);
  /* Infinite loop */
  for(;;)
  {
    process_encoder();
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

	UBaseType_t uxHighWaterMark, uxHighWaterMark_old;

	/* Inspect our own high water mark on entering the task. */
	uxHighWaterMark_old = uxTaskGetStackHighWaterMark( NULL );
	uxHighWaterMark = uxHighWaterMark_old;

  for(;;)
  {
	  xQueueReceive(qUSB_rcvQueue, &buf, portMAX_DELAY );
	  microrl_print_char(buf);
	  uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
	  if (uxHighWaterMark < uxHighWaterMark_old)
	  {
		  uxHighWaterMark_old = uxHighWaterMark;
	  }

  }
  /* USER CODE END StartUSB_rcv */
}

/* USER CODE BEGIN Header_StartUARTtask */
/**
* @brief Function implementing the UARTtask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUARTtask */
void StartUARTtask(void *argument)
{
  /* USER CODE BEGIN StartUARTtask */
  /* Infinite loop */

  uint8_t data;
  for(;;)
  {
	HAL_UART_Receive_IT(&huart2, &data, 1);
	/* Wait to be notified of an interrupt. */
	ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
	if (get_nema())
		CDC_Transmit_FS(&data, 1);
	//xQueueSend(qdebugRTTHandle, &data, portMAX_DELAY);
    //osDelay(1);
  }
  /* USER CODE END StartUARTtask */
}

/* USER CODE BEGIN Header_StartEncoder */
/**
* @brief Function implementing the Encoder thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartEncoder */
void StartEncoder(void *argument)
{
  /* USER CODE BEGIN StartEncoder */

	static bool invert = false;
	static bool released = true;

  osDelay(200);
  union VFD {
	  uint8_t arr2[11][3];
	  uint8_t arr1[11*3];
  } vfd;

  for (int i = 0; i < sizeof(vfd.arr1); i++)
  {
	  vfd.arr1[i] = 0;
  }
  uint8_t data;

  data = 0b01000000; // command 2, write to Display port
  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 0);
  HAL_SPI_Transmit(&hspi2, &data, 1, 0xffffffff);
  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 1);
  osDelay(10);
  data = 0b11000000; // command 3, set address to 0
  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 0);
  HAL_SPI_Transmit(&hspi2, &data, 1, 0xffffffff);


  HAL_SPI_Transmit(&hspi2, vfd.arr1, sizeof(vfd.arr1), 0xffffffff);

//  for (uint8_t i = 0; i < sizeof(vfd.arr1); i++)
//  {
//	  osDelay(10);
//  }
  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 1);
  osDelay(10);

  data = 0b11000000; // command 3, set address to 0
  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 0);
  HAL_SPI_Transmit(&hspi2, &data, 1, 0xffffffff);


  for (uint8_t i = 0; i < sizeof(vfd.arr1); i++)
  {
	  vfd.arr1[i] = 0xaa;
  }

  HAL_SPI_Transmit(&hspi2, vfd.arr1, sizeof(vfd.arr1), 0xffffffff);
  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 1);
  osDelay(10);
  // init display, 11 digits 17 segments
  data = 0b00000111; // command 1, 11 digits 17 segments
  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 0);
  HAL_SPI_Transmit(&hspi2, &data, 1, 0xffffffff);
  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 1);

  osDelay(10);
  data = 0b01000000; // command 2, write to Display port
  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 0);
  HAL_SPI_Transmit(&hspi2, &data, 1, 0xffffffff);
  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 1);
  osDelay(10);

  // write some data

  /* Infinite loop */
  for(;;)
  {

	  if (HAL_GPIO_ReadPin(enc_s_GPIO_Port, enc_s_Pin))
	  {
		  released = true;
	  } else if (released)
	  {
		  released = false;
		  invert = !invert;
	  }

	  data = 0b01000001; // command 2, write to LED port
	  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 0);
	  HAL_SPI_Transmit(&hspi2, &data, 1, 0xffffffff);
	  osDelay(10);

	  data = ~(1<<((encoder_value >> 2)&0b11));
	  if (invert)
		  data =~data;
	  HAL_GPIO_WritePin(HV_EN_GPIO_Port, HV_EN_Pin, invert);

	  HAL_SPI_Transmit(&hspi2, &data, 1, 0xffffffff);
	  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 1);

	  osDelay(10);
	  data = 0b10000000; // command 4
	  data |= invert<<3; // enable/disable display
	  data |= (encoder_value >> 2)&0b111; // set brightness
	  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 0);
	  HAL_SPI_Transmit(&hspi2, &data, 1, 0xffffffff);
	  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 1);
	  osDelay(10);
  }
  /* USER CODE END StartEncoder */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart != &huart2)
		return;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;


   /* Notify the task that the transmission is complete by setting the TX_BIT
   in the task's notification value. */
	vTaskNotifyGiveFromISR( UARTtaskHandle,
					   &xHigherPriorityTaskWoken );

   /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
   should be performed to ensure the interrupt returns directly to the highest
   priority task.  The macro used for this purpose is dependent on the port in
   use and may be called portEND_SWITCHING_ISR(). */
   portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void process_encoder(void)
{
	static uint8_t old;
	uint8_t new;
	new = (0b01*HAL_GPIO_ReadPin(enc_a_GPIO_Port, enc_a_Pin) +
		   0b10*HAL_GPIO_ReadPin(enc_b_GPIO_Port, enc_b_Pin));
	switch(old)
		{
		case 2:
			{
			if(new == 3) encoder_value++;
			if(new == 0) encoder_value--;
			break;
			}

		case 0:
			{
			if(new == 2) encoder_value++;
			if(new == 1) encoder_value--;
			break;
			}
		case 1:
			{
			if(new == 0) encoder_value++;
			if(new == 3) encoder_value--;
			break;
			}
		case 3:
			{
			if(new == 1) encoder_value++;
			if(new == 2) encoder_value--;
			break;
			}
		}
	old = new;
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
