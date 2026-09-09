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
#include "motor.h"
#include "pid.h"
#include "tim.h"   /* 使用 htim5 驱动流水灯 */
#include "synex.h" /* 上位机串口通信（JustFloat 上传 + 在线调参命令） */

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
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for TaskMotor */
osThreadId_t TaskMotorHandle;
const osThreadAttr_t TaskMotor_attributes = {
    .name = "TaskMotor",
    .stack_size = 1256 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTaskLEDFlowing(void *argument);
void StartTaskServo(void *argument);
void StartTaskMotor(void *argument);

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

  /* creation of TaskMotor */
  TaskMotorHandle = osThreadNew(StartTaskMotor, NULL, &TaskMotor_attributes);

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
  // osDelay(1000);
  // Buzzer_PlayErrorTone(BUZZER_ERR_1);
  // osDelay(800);
  // Buzzer_PlayErrorTone(BUZZER_ERR_2);

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
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  __HAL_TIM_MOE_ENABLE(&htim1); /* 高级定时器必须打开主输出(MOE)，否则 PWM 不输出 */
  for (;;)
  {
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 500); /* 0° */
    osDelay(1000);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 1000); /* 45° */
    osDelay(1000);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 1500); /* 90° */
    osDelay(1000);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 2000); /* 135° */
    osDelay(1000);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 2500); /* 180° */
    osDelay(1000);
  }
  /* USER CODE END StartTaskServo */
}

/* USER CODE BEGIN Header_StartTaskMotor */
/**
 * @brief Function implementing the TaskMotor thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTaskMotor */
void StartTaskMotor(void *argument)
{
  /* USER CODE BEGIN StartTaskMotor */

  /* 电机与串口初始化（只一次） */
  Motor_Init();
  Synex_Init();

  /* 角度环 PID 增益：初值可被 Synex 命令在线修改 */
  float kp_angle = 1.0f;
  float ki_angle = 0.5f;
  float kd_angle = 0.0f;

  /* 双环 PID：角度环(外) → 速度环(内) */
  PID_t angle_pid;
  PID_Init(&angle_pid, kp_angle, ki_angle, kd_angle, 200.0f);
  PID_t speed_pid;
  PID_Init(&speed_pid, 20.0f, 0.0f, 0.0f, 6000.0f);

  float target_pos = 0.0f;      /* 目标角度（精确两秒轨迹生成） */
  float last_target = 0.0f;     /* 上一次的目标角度（判断是否反向） */
  float last_motion_dir = 0.0f; /* 上一次目标移动方向 +1/-1/0 */
  uint32_t t0 = osKernelGetTickCount();
  uint32_t last_t = t0;
  uint32_t send_tick = 0U;      /* 串口上传节流计数 */

  /* Infinite loop */
  for (;;)
  {
    uint32_t now_t = osKernelGetTickCount();
    float dt = (float)(now_t - last_t) / 1000.0f; /* 距上一周期(秒) */
    last_t = now_t;
    float t = (float)(now_t - t0) / 1000.0f;

    /* 精确两秒轨迹：同时给目标角度和"目标速度前馈"
     * 0~1s：0°→90°，目标速度 +90°/s
     * 1~2s：90°→-90°，目标速度 -180°/s
     * 2s 后：保持 -90° */
    float traj_vel = 0.0f;
    if (t < 1.0f)
    {
      target_pos = 90.0f * t;
      traj_vel = 90.0f;
    }
    else if (t < 2.0f)
    {
      target_pos = 90.0f - 180.0f * (t - 1.0f);
      traj_vel = -180.0f;
    }
    else
    {
      target_pos = -90.0f;
      traj_vel = 0.0f;
    }

    /* 目标反向时清积分：避免往 +90° 积攒的正积分，反向时扯后腿 */
    float motion = target_pos - last_target;
    float motion_dir = (motion > 0.001f) ? 1.0f : ((motion < -0.001f) ? -1.0f : last_motion_dir);
    if ((last_motion_dir != 0.0f) && (motion_dir != last_motion_dir))
    {
      angle_pid.integral = 0.0f;
      angle_pid.prev_error = 0.0f;
      speed_pid.integral = 0.0f;
      speed_pid.prev_error = 0.0f;
    }
    last_motion_dir = motion_dir;
    last_target = target_pos;

    float now_angle = Motor_GetAngleDeg();       /* 当前输出轴角度(°) */
    float now_speed = Motor_GetSpeedDegPerSec(); /* 当前输出轴角速度(°/s) */

    /* 在线调参：若 Synex 发来 kp/ki/kd=X，更新增益 */
    {
      float val;
      uint8_t cmd = Synex_PollCommand(&val);
      if (cmd == 1u) { kp_angle = val; }
      else if (cmd == 2u) { ki_angle = val; }
      else if (cmd == 3u) { kd_angle = val; }
    }
    angle_pid.kp = kp_angle; /* 让新增益立刻生效 */
    angle_pid.ki = ki_angle;
    angle_pid.kd = kd_angle;

    /* 外环：角度 PID 纠偏 + 前馈目标速度（立即给足速度，不靠误差慢慢加） */
    angle_pid.target = target_pos;
    float target_speed = PID_Calc(&angle_pid, now_angle, dt) + traj_vel;

    /* 内环：转速误差 → 电流指令 */
    speed_pid.target = target_speed;
    float current = PID_Calc(&speed_pid, now_speed, dt);

    Motor_SendCurrent(0, (int16_t)current, 0, 0); /* 发给电机1（第2槽）电流 */

    /* 每 10ms 上传一帧 JustFloat 波形 */
    if (++send_tick >= 10U)
    {
      send_tick = 0U;
      Synex_Send(now_angle, target_pos, now_speed, (float)current,
                 kp_angle, ki_angle, kd_angle);
    }

    osDelay(1); /* 1kHz 控制周期 */
  }
  /* USER CODE END StartTaskMotor */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
