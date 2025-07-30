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
#include "comm_handler.h"  // CommStatus_t, PAYLOAD_SIZE 등 사용
#include "input_handler.h" // InputHandler_... 함수들 사용
#include "mpu6050.h"

extern I2C_HandleTypeDef hi2c2;
extern MPU6050_t MPU6050;

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
static float App_GetRollAngle(void);
static void App_BuildPacket(uint8_t* packet_buffer, float roll_angle);
static void App_HandleAckPayload(uint8_t* ack_payload);
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for commTask */
osThreadId_t commTaskHandle;
const osThreadAttr_t commTask_attributes = {
  .name = "commTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for sensorTask */
osThreadId_t sensorTaskHandle;
const osThreadAttr_t sensorTask_attributes = {
  .name = "sensorTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for ackHandlerTask */
osThreadId_t ackHandlerTaskHandle;
const osThreadAttr_t ackHandlerTask_attributes = {
  .name = "ackHandlerTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for sensorQueue */
osMessageQueueId_t sensorQueueHandle;
const osMessageQueueAttr_t sensorQueue_attributes = {
  .name = "sensorQueue"
};
/* Definitions for ackSem */
osSemaphoreId_t ackSemHandle;
const osSemaphoreAttr_t ackSem_attributes = {
  .name = "ackSem"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartcommTask(void *argument);
void StartsensorTask(void *argument);
void StartackHandlerTask(void *argument);

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
  /* creation of ackSem */
  ackSemHandle = osSemaphoreNew(1, 1, &ackSem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of sensorQueue */
  sensorQueueHandle = osMessageQueueNew (1, sizeof(float), &sensorQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of commTask */
  commTaskHandle = osThreadNew(StartcommTask, NULL, &commTask_attributes);

  /* creation of sensorTask */
  sensorTaskHandle = osThreadNew(StartsensorTask, NULL, &sensorTask_attributes);

  /* creation of ackHandlerTask */
  ackHandlerTaskHandle = osThreadNew(StartackHandlerTask, NULL, &ackHandlerTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartcommTask */
/**
  * @brief  Function implementing the commTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartcommTask */
void StartcommTask(void *argument)
{
  /* USER CODE BEGIN StartcommTask */
	  uint8_t tx_packet[PAYLOAD_SIZE] = {0};
	  float roll = 0.0f; // roll 값을 저장할 지역 변수

  /* Infinite loop */
  for(;;)
  {
	   osMessageQueueGet(sensorQueueHandle, &roll, NULL, osWaitForever);
	   App_BuildPacket(tx_packet, roll);
	   CommHandler_Transmit(tx_packet, PAYLOAD_SIZE);
  }
  /* USER CODE END StartcommTask */
}

/* USER CODE BEGIN Header_StartsensorTask */
/**
* @brief Function implementing the sensorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartsensorTask */
void StartsensorTask(void *argument)
{
  /* USER CODE BEGIN StartsensorTask */
  /* Infinite loop */
  for(;;)
  {
	    // 1. 센서 값을 읽어옵니다.
	    float roll = App_GetRollAngle();

	    // 2. 읽어온 값을 메시지 큐에 넣습니다.
	    osMessageQueuePut(sensorQueueHandle, &roll, 0U, 0U);

	    // 3. 5ms 주기로 실행합니다.
	    osDelay(5);

  }
  /* USER CODE END StartsensorTask */
}

/* USER CODE BEGIN Header_StartackHandlerTask */
/**
* @brief Function implementing the ackHandlerTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartackHandlerTask */
void StartackHandlerTask(void *argument)
{
  /* USER CODE BEGIN StartackHandlerTask */
	  uint8_t ack_packet[ACK_PAYLOAD_SIZE] = {0};

  /* Infinite loop */
  for(;;)
  {
	  // 1. 세마포어 신호가 올 때까지 무한정 대기 (휴면 상태, CPU 점유율 0)
	  osSemaphoreAcquire(ackSemHandle, osWaitForever);

	    // 2. 신호가 와서 깨어나면, ACK 처리 로직을 수행
	    CommStatus_t status = CommHandler_CheckStatus(ack_packet, ACK_PAYLOAD_SIZE);
	    if (status == COMM_TX_SUCCESS)
	    {
	        App_HandleAckPayload(ack_packet);
	    }
	    else if (status == COMM_TX_FAIL)
	    {
	        // TODO: 통신 실패 시 처리 로직
	    }
	  }
  /* USER CODE END StartackHandlerTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

// ★★★ main 루프의 상세 로직을 담당하는 내부 헬퍼 함수들 ★★★

/**
  * @brief  MPU6050 센서로부터 Roll 각도를 읽어옵니다.
  */
static float App_GetRollAngle(void)
{
    MPU6050_Read_All(&hi2c2, &MPU6050);
    return MPU6050.KalmanAngleX;
}

/**
  * @brief  각 모듈에서 데이터를 가져와 전송 패킷을 구성합니다.
  */
static void App_BuildPacket(uint8_t* packet_buffer, float roll_angle)
{
    memset(packet_buffer, 0, PAYLOAD_SIZE);

    packet_buffer[0] = 1; // ID

    // Roll 값 인코딩
    int16_t roll_encoded = (int16_t)(roll_angle * 100.0f);
    memcpy(&packet_buffer[1], &roll_encoded, sizeof(int16_t));

    // 버튼 입력 시간 가져오기
    uint16_t accel_ms = InputHandler_GetAccelMillis();
    uint16_t brake_ms = InputHandler_GetBrakeMillis();
    memcpy(&packet_buffer[3], &accel_ms, sizeof(uint16_t));
    memcpy(&packet_buffer[5], &brake_ms, sizeof(uint16_t));

    // 방향 정보 가져오기
    packet_buffer[7] = InputHandler_GetDirection();
}

/**
  * @brief  수신된 ACK 페이로드를 처리합니다.
  */
static void App_HandleAckPayload(uint8_t* ack_payload)
{
    if (ack_payload[0] == 1)
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
}
/* USER CODE END Application */

