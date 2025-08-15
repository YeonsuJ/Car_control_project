/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include "motor_control.h"
#include "can_handler.h"
#include "rf_handler.h"
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
/* Definitions for RFTask */
osThreadId_t RFTaskHandle;
const osThreadAttr_t RFTask_attributes = {
  .name = "RFTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for CANTask */
osThreadId_t CANTaskHandle;
const osThreadAttr_t CANTask_attributes = {
  .name = "CANTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for CANTxQueue */
osMessageQueueId_t CANTxQueueHandle;
const osMessageQueueAttr_t CANTxQueue_attributes = {
  .name = "CANTxQueue"
};
/* Definitions for CANRxQueue */
osMessageQueueId_t CANRxQueueHandle;
const osMessageQueueAttr_t CANRxQueue_attributes = {
  .name = "CANRxQueue"
};
/* Definitions for RFSem */
osSemaphoreId_t RFSemHandle;
const osSemaphoreAttr_t RFSem_attributes = {
  .name = "RFSem"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartRFTask(void *argument);
void StartCANTask(void *argument);

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

  /* Create the semaphores(s) */
  /* creation of RFSem */
  RFSemHandle = osSemaphoreNew(1, 0, &RFSem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of CANTxQueue */
  CANTxQueueHandle = osMessageQueueNew (2, sizeof(VehicleCommand_t), &CANTxQueue_attributes);

  /* creation of CANRxQueue */
  CANRxQueueHandle = osMessageQueueNew (1, sizeof(uint8_t), &CANRxQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of RFTask */
  RFTaskHandle = osThreadNew(StartRFTask, NULL, &RFTask_attributes);

  /* creation of CANTask */
  CANTaskHandle = osThreadNew(StartCANTask, NULL, &CANTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartRFTask */
/**
  * @brief  Function implementing the RFTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartRFTask */
void StartRFTask(void *argument)
{
  /* USER CODE BEGIN StartRFTask */
	VehicleCommand_t cmd = {0};
	uint8_t can_distance_signal = 0;

  /* Infinite loop */
	for(;;)
	  {
	      // 1. RF 수신 인터럽트가 발생할 때까지 대기
		  osSemaphoreAcquire(RFSemHandle, osWaitForever);

	      // 2. CAN 수신 큐에서 최신 거리 값을 논블로킹으로 확인
		  osMessageQueueGet(CANRxQueueHandle, &can_distance_signal, NULL, 0U);

	      // 3. RF 수신 버퍼에 있는 모든 명령을 처리
		  while (RFHandler_GetNewCommand(&cmd)) {
	          // 4. 모터 제어 업데이트
			  MotorControl_Update(&cmd);

	          // 5. CAN 전송을 위해 수신한 cmd 구조체 전체를 CANTxQueue에 넣음
			  osMessageQueuePut(CANTxQueueHandle, &cmd, 0U, 0U);

	          // 6. 컨트롤러에 보낼 ACK 페이로드에 거리 값 설정
			  RFHandler_SetAckPayload(can_distance_signal);
		  }
	  }
  /* USER CODE END StartRFTask */
}

/* USER CODE BEGIN Header_StartCANTask */
/**
* @brief Function implementing the CANTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCANTask */
void StartCANTask(void *argument)
{
  /* USER CODE BEGIN StartCANTask */
	VehicleCommand_t received_cmd; // ⭐️ VehicleCommand_t 타입으로 변경

  /* Infinite loop */
  for(;;)
  {
      // 큐에서 VehicleCommand_t 구조체 수신
	  osMessageQueueGet(CANTxQueueHandle, &received_cmd, NULL, osWaitForever);

      // 수신한 구조체에서 필요한 데이터 추출
      uint8_t dir = received_cmd.direction;
      uint8_t brake = (received_cmd.brake_ms > 0) ? 1 : 0;

      // 실제 CAN 전송 함수 호출
	  CAN_Send_DriveStatus(dir, brake);
  }
  /* USER CODE END StartCANTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

