/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    synex.h
  * @brief   上位机串口通信模块（Synex/VOFA+ 可视化）
  *
  * 功能：
  *   1. JustFloat 协议上传 7 通道波形到上位机
  *   2. 接收上位机下发的 "kp=X / ki=X / kd=X" 命令（在线调参）
  * 依赖：USART1（PA9/PA10 或工程配置引脚），波特率 115200
  ******************************************************************************
  */
/* USER CODE END Header */
#ifndef __SYNEX_H
#define __SYNEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Prototypes */

/* 启动串口接收（进入控制任务前调用一次） */
void Synex_Init(void);

/* 发送一帧 JustFloat：7 通道（角度/目标/转速/电流/kp/ki/kd）+ 帧尾 */
void Synex_Send(float angle, float target, float speed,
                float current, float kp, float ki, float kd);

/* 轮询接收到的命令：
 *   返回 0 = 无新命令
 *   返回 1/2/3 = 收到 kp=/ki=/kd=，*value 回传数值 */
uint8_t Synex_PollCommand(float *value);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /* __SYNEX_H */
