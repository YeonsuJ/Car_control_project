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
#include "can_handler.h"
#include "ultrasonic.h"
#include "motor_encoder.h"
#include "tim.h"
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
/* Definitions for SensorTask */
osThreadId_t SensorTaskHandle;
const osThreadAttr_t SensorTask_attributes = {
  .name = "SensorTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for CANTask */
osThreadId_t CANTaskHandle;
const osThreadAttr_t CANTask_attributes = {
  .name = "CANTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for CANTxQueue */
osMessageQueueId_t CANTxQueueHandle;
const osMessageQueueAttr_t CANTxQueue_attributes = {
  .name = "CANTxQueue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartSensorTask(void *argument);
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

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of CANTxQueue */
  CANTxQueueHandle = osMessageQueueNew (4, sizeof(SensorData_t), &CANTxQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of SensorTask */
  SensorTaskHandle = osThreadNew(StartSensorTask, NULL, &SensorTask_attributes);

  /* creation of CANTask */
  CANTaskHandle = osThreadNew(StartCANTask, NULL, &CANTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartSensorTask */
/**
  * @brief  Function implementing the SensorTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartSensorTask */
void StartSensorTask(void *argument)
{
  /* USER CODE BEGIN StartSensorTask */
	// --- SensorTask 초기화 ---
	Ultrasonic_Init();
	HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);

	// osDelayUntil을 사용하기 위한 변수
	uint32_t last_wake_time = osKernelGetTickCount();
	const uint32_t period_ms = 10; // 10ms 주기
  /* Infinite loop */
	for(;;)
	  {
	    // <<-- 2. 수정된 부분: osDelayUntil 사용법 (CMSIS-RTOS V2) -->>
	    // 다음 깨어날 시간을 계산한 후, 그 시간까지 Task를 지연시킵니다.
	    last_wake_time += period_ms;
	    osDelayUntil(last_wake_time);

	    // 센서 데이터 수집
	    Update_Motor_RPM();
	    Ultrasonic_Trigger();

	    SensorData_t sensor_packet; // 전송할 데이터 패킷

	    // 데이터 패킷 채우기
	    sensor_packet.light_condition = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3);
	    sensor_packet.rpm = MotorControl_GetRPM();

	    // ISR과 공유하는 변수 보호 (Critical Section)
	    taskENTER_CRITICAL();
	    sensor_packet.distance_front = distance_front;
	    sensor_packet.distance_rear = distance_rear;
	    taskEXIT_CRITICAL();

	    // 패킷을 Queue로 전송
	    osMessageQueuePut(CANTxQueueHandle, &sensor_packet, 0, 0);
	  }
  /* USER CODE END StartSensorTask */
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
    // --- CANTask 초기화 ---
    CAN_tx_Init();

    SensorData_t received_packet; // Queue에서 받을 데이터 패킷

    /* Infinite loop */
    for(;;)
    {
        // 1. Queue에 데이터가 들어올 때까지 Block 상태로 대기
        if (osMessageQueueGet(CANTxQueueHandle, &received_packet, NULL, osWaitForever) == osOK)
        {
            // 2. 받은 패킷으로 CAN 전송 데이터(TxData) 가공 (수정된 로직)
            TxData[0] = (received_packet.distance_front <= 10 || received_packet.distance_rear <= 10);
            TxData[1] = (received_packet.light_condition == GPIO_PIN_SET);
            TxData[2] = (uint8_t)received_packet.rpm;

            // 3. CAN 메시지 전송
            CAN_Send();
        }
    }
    /* USER CODE END StartCANTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

