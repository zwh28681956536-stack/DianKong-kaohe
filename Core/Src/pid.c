/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    pid.c
  * @brief   位置式 PID 控制器
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "pid.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

void PID_Init(PID_t *pid, float kp, float ki, float kd, float out_limit)
{
  pid->kp = kp;
  pid->ki = ki;
  pid->kd = kd;
  pid->target = 0.0f;
  pid->integral = 0.0f;
  pid->prev_error = 0.0f;
  pid->out_limit = out_limit;
}

float PID_Calc(PID_t *pid, float current)
{
  float error = pid->target - current;   /* 误差 = 目标 - 当前 */

  /* 积分项（带积分限幅，防止积分饱和） */
  pid->integral += error;
  float i_max = (pid->ki > 0.001f) ? (pid->out_limit / pid->ki) : 0.0f;
  if (pid->integral >  i_max) pid->integral =  i_max;
  if (pid->integral < -i_max) pid->integral = -i_max;

  /* 微分项 */
  float diff = error - pid->prev_error;
  pid->prev_error = error;

  /* 输出 = 比例 + 积分 + 微分 */
  float out = pid->kp * error + pid->ki * pid->integral + pid->kd * diff;

  /* 输出限幅 */
  if (out >  pid->out_limit) out =  pid->out_limit;
  if (out < -pid->out_limit) out = -pid->out_limit;

  return out;
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
