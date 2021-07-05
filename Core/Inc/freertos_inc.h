/*
 * freertos_inc.h
 *
 *  Created on: Jun 22, 2021
 *      Author: makso
 */

#ifndef INC_FREERTOS_INC_H_
#define INC_FREERTOS_INC_H_

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "queue.h"

osMessageQueueId_t qUSB_rcvQueue;


#endif /* INC_FREERTOS_INC_H_ */
