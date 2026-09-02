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
  pid->p_limit = out_limit;   /* P 项限幅默认等于总限幅 */
  pid->d_limit = out_limit;   /* D 项限幅默认等于总限幅 */
}

void PID_SetTermLimit(PID_t *pid, float p_limit, float d_limit)
{
  pid->p_limit = p_limit;
  pid->d_limit = d_limit;
}

float PID_Calc(PID_t *pid, float current)
{
  float error = pid->target - current;   /* 误差 = 目标 - 当前 */

  /* 积分项（带积分限幅，防止积分饱和） */
  pid->integral += error;
  float i_max = (pid->ki > 0.001f) ? (pid->out_limit / pid->ki) : 0.0f;
  if (pid->integral >  i_max) pid->integral =  i_max;
  if (pid->integral < -i_max) pid->integral = -i_max;

  float diff = error - pid->prev_error;
  pid->prev_error = error;

  /* 比例项 P（单独限幅） */
  float p_out = pid->kp * error;
  if (p_out >  pid->p_limit) p_out =  pid->p_limit;
  if (p_out < -pid->p_limit) p_out = -pid->p_limit;

  /* 微分项 D（单独限幅） */
  float d_out = pid->kd * diff;
  if (d_out >  pid->d_limit) d_out =  pid->d_limit;
  if (d_out < -pid->d_limit) d_out = -pid->d_limit;

  /* 总输出 = P + I + D */
  float out = p_out + pid->ki * pid->integral + d_out;

  /* 总输出限幅（兜底，保护电机） */
  if (out >  pid->out_limit) out =  pid->out_limit;
  if (out < -pid->out_limit) out = -pid->out_limit;

  return out;
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
