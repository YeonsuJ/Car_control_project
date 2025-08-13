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
osThreadId RFTaskHandle;
osThreadId CANTaskHandle;
osMutexId ackPayloadMutexHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartRFTask(void const * argument);
void StartCANTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* definition and creation of ackPayloadMutex */
  osMutexDef(ackPayloadMutex);
  ackPayloadMutexHandle = osMutexCreate(osMutex(ackPayloadMutex));

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
  /* definition and creation of RFTask */
  osThreadDef(RFTask, StartRFTask, osPriorityHigh, 0, 256);
  RFTaskHandle = osThreadCreate(osThread(RFTask), NULL);

  /* definition and creation of CANTask */
  osThreadDef(CANTask, StartCANTask, osPriorityNormal, 0, 128);
  CANTaskHandle = osThreadCreate(osThread(CANTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartRFTask */
/**
  * @brief  Function implementing the RFTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartRFTask */
void StartRFTask(void const * argument)
{
  /* USER CODE BEGIN StartRFTask */
	VehicleCommand_t cmd = {0};
  /* Infinite loop */
  for(;;)
  {
  // 새 RF 명령이 있는지 확인
	  if (RFHandler_GetNewCommand(&cmd, ackPayloadMutexHandle))
	  {
		  // 새 명령 수신 시 모터/서보 제어 및 RPM 업데이트 동시 수행
		  MotorControl_Update(&cmd);

		  // 전진/후진 + 브레이크 상태를 CAN으로 전송
		  uint8_t dir = cmd.direction;
		  uint8_t brake = (cmd.brake_ms > 0) ? 1 : 0;
		  CAN_Send_DriveStatus(dir, brake);
	  }
	  osDelay(5); // CPU 점유율을 낮추기 위해 짧은 지연
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
void StartCANTask(void const * argument)
{
  /* USER CODE BEGIN StartCANTask */
  /* Infinite loop */
  for(;;)
  {
  // CAN 신호를 받아 다음 ACK 페이로드에 실릴 데이터를 설정
  // can_distance_signal은 CAN 수신 콜백에서 'volatile'로 업데이트됨
  RFHandler_SetAckPayload(can_distance_signal, ackPayloadMutexHandle);
  osDelay(20); // 20ms 주기로 ACK 페이로드 업데이트
  }
  /* USER CODE END StartCANTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

