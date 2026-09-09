/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    synex.c
  * @brief   上位机串口通信模块（Synex/VOFA+ 可视化）
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "synex.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>

/* USER CODE BEGIN 0 */

#define SYNEX_CMD_BUF_SIZE  24u   /* 命令行缓冲长度 */

/* 串口接收状态 */
static uint8_t  rx_byte;           /* 单字节接收缓冲 */
static char     cmd_buf[SYNEX_CMD_BUF_SIZE]; /* 一行命令缓冲 */
static uint8_t  cmd_len;           /* 已收字节数 */
static volatile uint8_t cmd_ready; /* 一行完整且解析出数值的标志 */
static uint8_t  cmd_id;            /* 命令类型：1=kp 2=ki 3=kd */
static float    cmd_value;         /* 命令携带的数值 */

/* USER CODE END 0 */

/* 启动串口接收：进入接收中断，逐字节收 */
void Synex_Init(void)
{
  cmd_len = 0u;
  cmd_ready = 0u;
  HAL_UART_Receive_IT(&huart1, &rx_byte, 1u);
}

/* 发送一帧 JustFloat：7 个 float32(小端) + 帧尾 0x7F800000(+inf) */
void Synex_Send(float angle, float target, float speed,
                float current, float kp, float ki, float kd)
{
  uint32_t frame[8];
  memcpy(&frame[0], &angle, 4);
  memcpy(&frame[1], &target, 4);
  memcpy(&frame[2], &speed, 4);
  memcpy(&frame[3], &current, 4);
  memcpy(&frame[4], &kp, 4);
  memcpy(&frame[5], &ki, 4);
  memcpy(&frame[6], &kd, 4);
  frame[7] = 0x7F800000u;  /* JustFloat 帧尾 = +inf */
  HAL_UART_Transmit(&huart1, (uint8_t *)frame, sizeof(frame), 10u);
}

/* 轮询：有新命令返回 1/2/3（kp/ki/kd）并回传数值，否则返回 0 */
uint8_t Synex_PollCommand(float *value)
{
  if (cmd_ready != 0u)
  {
    cmd_ready = 0u;
    *value = cmd_value;
    return cmd_id;
  }
  return 0u;
}

/* 解析一行命令："kp=0.1" / "ki=0.2" / "kd=0.01"，成功则记录 id 与数值 */
static void Synex_ParseLine(void)
{
  float v;
  cmd_id = 0u;

  if ((strncmp(cmd_buf, "kp=", 3u) == 0) && (sscanf(cmd_buf + 3, "%f", &v) == 1))
  {
    cmd_id = 1u;
  }
  else if ((strncmp(cmd_buf, "ki=", 3u) == 0) && (sscanf(cmd_buf + 3, "%f", &v) == 1))
  {
    cmd_id = 2u;
  }
  else if ((strncmp(cmd_buf, "kd=", 3u) == 0) && (sscanf(cmd_buf + 3, "%f", &v) == 1))
  {
    cmd_id = 3u;
  }

  if (cmd_id != 0u)
  {
    cmd_value = v;
    cmd_ready = 1u;
  }
}

/* 串口接收中断回调：把字节攒成一行，遇 \r 或 \n 表示一行结束并解析 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart != &huart1)
  {
    return;
  }

  char c = (char)rx_byte;
  if (c == '\r' || c == '\n')
  {
    if (cmd_len > 0u)
    {
      cmd_buf[cmd_len] = '\0';
      Synex_ParseLine();
    }
    cmd_len = 0u;
  }
  else if (cmd_len < (SYNEX_CMD_BUF_SIZE - 1u))
  {
    cmd_buf[cmd_len++] = c;
  }

  HAL_UART_Receive_IT(&huart1, &rx_byte, 1u);  /* 继续收下一个字节 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
