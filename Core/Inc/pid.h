/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    pid.h
  * @brief   位置式 PID 控制器（角度闭环用）
  ******************************************************************************
  */
/* USER CODE END Header */
#ifndef __PID_H
#define __PID_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* PID 结构体 */
typedef struct {
  float kp;         /* 比例系数 */
  float ki;         /* 积分系数 */
  float kd;         /* 微分系数 */
  float target;     /* 目标值（角度） */
  float integral;   /* 积分累计 */
  float prev_error; /* 上次误差（算微分用） */
  float out_limit;  /* 输出限幅（防止电流过大） */
} PID_t;

/* 初始化 PID：kp/ki/kd 和输出限幅 */
void PID_Init(PID_t *pid, float kp, float ki, float kd, float out_limit);

/* 计算一次 PID 输出：传入当前值（角度），返回输出（电流指令） */
float PID_Calc(PID_t *pid, float current);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /* __PID_H */
