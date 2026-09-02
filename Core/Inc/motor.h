/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    motor.h
  * @brief   M3508 电机驱动（通过 C620 电调，CAN2 总线）
  *
  * 协议：
  *   发送：CAN ID 0x200，8 字节 = 4 个电机的电流指令（-16384~16384）
  *   接收：CAN ID 0x201~0x204，8 字节 = 转速/角度/电流/温度
  ******************************************************************************
  */
/* USER CODE END Header */
#ifndef __MOTOR_H
#define __MOTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* 电机数据结构体：C620 反馈的数据 */
typedef struct {
  int16_t  speed_rpm;       /* 转子转速(rpm) */
  int16_t  torque_current;  /* 实际电流 */
  uint16_t angle_raw;       /* 转子角度原始值(0~8191) */
  uint8_t  online;          /* 1=在线（收到过反馈） */
} Motor_t;

/* 4 个电机的档案（motor[0] 对应 ID 0x201，即第一个电机） */
extern Motor_t motor[4];

/* 初始化 CAN 过滤器、启动 CAN、打开接收中断。须在 MX_CAN2_Init() 之后调用一次 */
void Motor_Init(void);

/* 发送电流指令到 4 个电机（c1~c4 范围 -16384~+16384） */
void Motor_SendCurrent(int16_t c1, int16_t c2, int16_t c3, int16_t c4);

/* 读取电机0 的输出轴角度(°)，已按减速比换算并处理多圈回绕 */
float Motor_GetAngleDeg(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /* __MOTOR_H */
