/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "main.h"
#include "task.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "buzzer.h"
#include "tim.h" /* 使用 htim5 驱动流水灯 */
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
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for TaskLEDFlowing */
osThreadId_t TaskLEDFlowingHandle;
const osThreadAttr_t TaskLEDFlowing_attributes = {
    .name = "TaskLEDFlowing",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for TaskServo */
osThreadId_t TaskServoHandle;
const osThreadAttr_t TaskServo_attributes = {
    .name = "TaskServo",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityBelowNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTaskLEDFlowing(void *argument);
void StartTaskServo(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
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

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of TaskLEDFlowing */
  TaskLEDFlowingHandle = osThreadNew(StartTaskLEDFlowing, NULL, &TaskLEDFlowing_attributes);

  /* creation of TaskServo */
  TaskServoHandle = osThreadNew(StartTaskServo, NULL, &TaskServo_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
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
  /* USER CODE BEGIN StartDefaultTask */

  /* 报错音调调用示例（可删除）：
   * 实际使用中，在检测到对应故障时调用即可：
   *   Buzzer_PlayErrorTone(BUZZER_ERR_1);   // 报错音调 1：小星星
   *   Buzzer_PlayErrorTone(BUZZER_ERR_2);   // 报错音调 2：欢乐颂
   * 下方演示：上电后各播放一次，便于听音确认两种音调正常。
   */
  osDelay(1000);
  Buzzer_PlayErrorTone(BUZZER_ERR_1);
  osDelay(800);
  Buzzer_PlayErrorTone(BUZZER_ERR_2);

  /* Infinite loop */
  for (;;)
  {
    /* ── 错误检测区：每圈检查一遍 ── */
    /* 其他模块检测到故障时，把对应标志置 1，这里就会播放报错音调：
     *   Buzzer_ErrFlag1 = 1;  → 播放《小星星》（报错音调 1）
     *   Buzzer_ErrFlag2 = 1;  → 播放《欢乐颂》（报错音调 2） */
    if (Buzzer_ErrFlag1 != 0U)
    {
      Buzzer_PlayErrorTone(BUZZER_ERR_1);
      Buzzer_ErrFlag1 = 0U;
    }
    if (Buzzer_ErrFlag2 != 0U)
    {
      Buzzer_PlayErrorTone(BUZZER_ERR_2);
      Buzzer_ErrFlag2 = 0U;
    }
    osDelay(10);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTaskLEDFlowing */
/**
 * @brief Function implementing the TaskLEDFlowing thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTaskLEDFlowing */
void StartTaskLEDFlowing(void *argument)
{
  /* USER CODE BEGIN StartTaskLEDFlowing */
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_3);
  /* Infinite loop */
  for (;;)
  {
    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, 999);
    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_2, 0);
    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_3, 0);
    osDelay(300);

    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_2, 999);
    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_3, 0);
    osDelay(300);

    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_2, 0);
    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_3, 999);
    osDelay(300);
  }
  /* USER CODE END StartTaskLEDFlowing */
}

/* USER CODE BEGIN Header_StartTaskServo */
/**
 * @brief Function implementing the TaskServo thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTaskServo */
void StartTaskServo(void *argument)
{
  /* USER CODE BEGIN StartTaskServo */
  /* Infinite loop */
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  for (;;)
  {
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 500); /* 0° */
    osDelay(1000);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 1000); /* 45° */
    osDelay(1000);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 1500); /* 90° */
    osDelay(1000);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 2000); /* 135° */
    osDelay(1000);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 2500); /* 180° */
    osDelay(1000);
  }
  /* USER CODE END StartTaskServo */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
