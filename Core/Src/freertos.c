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
#include "spi.h"
#include "vfd.h"
#include "i2c.h"
#include "d3231.h"
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
uint16_t tick_counter = 0;

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
/* Definitions for qVFD */
osMessageQueueId_t qVFDHandle;
const osMessageQueueAttr_t qVFD_attributes = {
  .name = "qVFD"
};
/* Definitions for muI2C */
osMutexId_t muI2CHandle;
const osMutexAttr_t muI2C_attributes = {
  .name = "muI2C"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
#if USE_ENCODER
void process_encoder(void);
#endif

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartLEDheartbeat(void *argument);
void StartUSB_rcv(void *argument);
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
  /* Create the mutex(es) */
  /* creation of muI2C */
  muI2CHandle = osMutexNew(&muI2C_attributes);

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
  qUSB_rcvHandle = osMessageQueueNew (32, sizeof(uint8_t), &qUSB_rcv_attributes);

  /* creation of qVFD */
  qVFDHandle = osMessageQueueNew (16, sizeof(uint16_t), &qVFD_attributes);

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
#if USE_ENCODER
    process_encoder();
#endif
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

		if (use_leds)
			HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		else
			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, 1);

		tick_counter++;

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

	static bool invert = true;
	static bool released = true;


	void vfd_update(void)
	{
		uint8_t data = 0b11000000; // command 3, set address to 0
	    HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 0);
	    HAL_SPI_Transmit(&hspi2, &data, 1, 0xffffffff);
	    HAL_SPI_Transmit(&hspi2, vfd.arr1, sizeof(vfd.arr1), 0xffffffff);
	    HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 1);
	}


  osDelay(500);
  HAL_GPIO_WritePin(HV_EN_GPIO_Port, HV_EN_Pin, 1);



  for (int i = 0; i < sizeof(vfd.arr1); i++)
  {
	  vfd.arr1[i] = 0xFF;
  }
  uint8_t data;

  data = 0b01000001; // command 2, write to LED port
  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 0);
  HAL_SPI_Transmit(&hspi2, &data, 1, 0xffffffff);
  osDelay(10);

  data = 0b1111; // disable LEDs

  HAL_SPI_Transmit(&hspi2, &data, 1, 0xffffffff);
  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 1);


  data = 0b01000000; // command 2, write to Display port
  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 0);
  HAL_SPI_Transmit(&hspi2, &data, 1, 0xffffffff);
  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 1);
  osDelay(10);
  vfd_update();
  osDelay(10);
  // init display, 11 digits 17 segments
  data = 0b00000111; // command 1, 11 digits 17 segments
  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 0);
  HAL_SPI_Transmit(&hspi2, &data, 1, 0xffffffff);
  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 1);
  osDelay(10);

  for (uint8_t i = 0; i <= 0b111; i++)
  {
	  data = 0b10000000; // command 4
	  data |= 1<<3; // enable/disable display
	  data |= i; // set brightness
	  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 0);
	  HAL_SPI_Transmit(&hspi2, &data, 1, 0xffffffff);
	  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 1);
	  osDelay(250);
  }



  for (int i = 0; i < 11; i++)
  {
	  for (int b = 0; b < 3; b++) // erasing from right to left
	  {
		  vfd.arr2[i][b] = 0;
	  }
	  vfd_update();
	  osDelay(150);
  }
  osDelay (500);

  //erase everything... just in case
  clr_vfd();


  // fill everything
    for (int j = 1; j < 15; j++)
    {
  	  uint32_t temp = 1<<j;
  	  for (int i = 1; i < 11; i++)
  	  {
  		  for (int b = 0; b < 3; b++)
  		  {
  			  vfd.arr2[i][b] |= (temp>>(b<<3))&0xFF;
  		  }
  	  }
  	  vfd_update();
  	  osDelay(100);
    }

    const uint8_t arr[][2] = {{6, 0},
    				   {0, 0},
					   {0, 1},
					   {0, 4},
					   {0, 3},
					   {0, 5},
					   {0, 2},
					   {0, 6},
					   {1, 16},
					   {1, 15},
					   {2, 16},
					   {2, 15},
					   {3, 16},
					   {3, 15},
					   {4, 16},
					   {4, 15},
					   {5, 16},
					   {5, 15},
					   {6, 16},
					   {6, 15},
					   {8, 16},
					   {8, 15},
					   {9, 16},
					   {10, 16},
					   {10, 15},
    };

    for (int j = 0; j < sizeof(arr)/2; j++)
    {
		for (int b = 0; b < 3; b++)
		  vfd.arr2[arr[j][0]][b] |= ((1<<arr[j][1])>>(b<<3))&0xFF;
		vfd_update();
		osDelay(70);
    }

    osDelay(500);

    //erase everything... just in case
    clr_vfd();

    vfd_update();


	const char * demo = "VFD FV651G";
	while (*demo)
	{
		uint16_t temp = get_char(*(demo++));
		xQueueSendToBack(qVFDHandle, &temp, 100);
	}


  d3231_get_all();

  uint8_t brightness = 0b111-d3231_get_A2M2(); // alarm2 minutes as EEPROM, default max

  data = 0b10000000; // command 4
  data |= 1<<3; // enable/disable display
  data |= brightness&0b111; // set brightness
  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 0);
  HAL_SPI_Transmit(&hspi2, &data, 1, 0xffffffff);
  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 1);

  /* Infinite loop */
  for(;;)
  {
	  uint16_t buf;
	  // show temperature
	  if (HAL_GPIO_ReadPin(PB1_GPIO_Port, PB1_Pin))
	  {
		  //erase everything...
		  clr_vfd();

		  uint8_t td3231 = *d3231_get_temp();
		  uint8_t td [6];
		  td[0] = 'C';
		  td[1] = 176; //°
		  uint8_t i = 2;
		  while (td3231)
		  {
			  td[i++] = td3231 %10;
			  td3231 /= 10;
		  }
		  if (i>2)
			  td[i] = td3231&(1<<7)?'-':'+';

		  for (int i = 0; i < 6; i++)
		  {
			  buf = get_char(td[i]);

			  vfd.arr2[i+1][0] = buf & 0xFF;
			  vfd.arr2[i+1][1] = (buf>>8)&0xFF;
		  }

		  vfd_update();
		  osDelay(20);
		  while(HAL_GPIO_ReadPin(PB1_GPIO_Port, PB1_Pin)); // wait release
		  osDelay(1000);
		  show_clock = true;
	  }

	  // tune brightness
	  if (HAL_GPIO_ReadPin(PB2_GPIO_Port, PB2_Pin))
	  {
		  brightness = (brightness - 1)&0b111;
		  d3231_set_A2M2(0b111-brightness);

		  save_vfd();
		  clr_vfd();
		  uint32_t bits = 0;
		  for (int i = 2; i < 1 + 2 + brightness; i++)
			  bits |= 1<<i;
		  symbols_vfd(bits);
		  str2vfd("brightness");
		  vfd_update();

		  data = 0b10000000; // command 4
		  data |= 1<<3; // enable/disable display
		  data |= brightness&0b111; // set brightness
		  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 0);
		  HAL_SPI_Transmit(&hspi2, &data, 1, 0xffffffff);
		  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 1);
		  // todo display BRIGHTNESS and scale
		  osDelay(20);
		  while(HAL_GPIO_ReadPin(PB2_GPIO_Port, PB2_Pin)); // wait release
		  osDelay(100);
		  restore_vfd();
		  vfd_update();
	  }

	  if (show_clock)
	  {
		  uint8_t * time = d3231_get_time();
		  uint8_t clock [4];
		  clock[0] = time[1] & 0xF;
		  clock[1] = (time[1] >> 4) & 0xF;
		  clock[2] = time[2] & 0xF;
		  clock[3] = (time[2] >> 4) & 0xF;

		  //erase everything...
		  clr_vfd();


		  for (int i = 0; i < 4; i++)
		  {
			  buf = get_char(clock[i]);

			  vfd.arr2[4+i][0] = buf & 0xFF;
			  vfd.arr2[4+i][1] = (buf>>8)&0xFF;
		  }

		  if ((time[0]&0b1) == 0)
		  {
				for (int b = 0; b < 3; b++)
				  vfd.arr2[6][b] |= ((1<<0)>>(b<<3))&0xFF;
		  }

		  vfd_update();


	  }
	  else
	  {
		  if(qVFDHandle && xQueueReceive(qVFDHandle, &buf, 1))
		  {
				for (int i = 10; i > 1; i--)
				{
					vfd.arr2[i][0] = vfd.arr2[i-1][0];
					vfd.arr2[i][1] = vfd.arr2[i-1][1];
				}
				vfd.arr2[1][0] = buf & 0xFF;
				vfd.arr2[1][1] = (buf>>8)&0xFF;
				vfd_update();
		  }
	  }

	  if (HAL_GPIO_ReadPin(enc_s_GPIO_Port, enc_s_Pin))
	  {
		  released = true;
	  } else if (released)
	  {
		  released = false;
		  invert = !invert;
	  }

	if(use_leds)
	{
		  data = 0b01000001; // command 2, write to LED port
		  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 0);
		  HAL_SPI_Transmit(&hspi2, &data, 1, 0xffffffff);
		  osDelay(10);

		  data = ~(1<<((tick_counter >> 1)&0b11));
	//	  if (invert)
	//		  data =~data;
		  HAL_GPIO_WritePin(HV_EN_GPIO_Port, HV_EN_Pin, invert);

		  HAL_SPI_Transmit(&hspi2, &data, 1, 0xffffffff);
		  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 1);
	}

#if USE_ENCODER
	  osDelay(10);
	  data = 0b10000000; // command 4
	  data |= invert<<3; // enable/disable display
	  data |= ((encoder_value >> 2) - 1)&0b111; // set brightness
	  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 0);
	  HAL_SPI_Transmit(&hspi2, &data, 1, 0xffffffff);
	  HAL_GPIO_WritePin(PT6315_STB_GPIO_Port, PT6315_STB_Pin, 1);
#endif
	  osDelay(10);
  }
  /* USER CODE END StartEncoder */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

#if USE_ENCODER
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
#endif
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
