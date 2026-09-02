/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    motor.c
  * @brief   M3508 电机驱动（通过 C620 电调，CAN2 总线）
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "motor.h"
#include "can.h"
#include "main.h"

/* USER CODE BEGIN 0 */

/* 减速比（M3508 默认 359:1，实际以你的电机为准） */
#define MOTOR_GEAR_RATIO      359.09f

/* 编码器一圈的计数（13 位，0~8191） */
#define MOTOR_ENCODER_PER_REV 8192u

Motor_t motor[4];

/* 角度解算状态：累加转子转过的圈数，从而得到输出轴的多圈绝对角度 */
static int32_t  wrap_count  = 0;
static uint16_t last_raw    = 0;
static uint8_t  angle_inited = 0;

/* 判断转子角度是否跨过了 0 点，累加圈数（用于多圈角度） */
static void Motor_UnwrapAngle(uint16_t raw)
{
  if (angle_inited == 0U) {
    last_raw = raw;
    angle_inited = 1U;
    return;
  }
  int16_t delta = (int16_t)(raw - last_raw);
  if (delta > 4096) {
    wrap_count--;         /* 从高值跳到低值：反向过了一圈 */
  } else if (delta < -4096) {
    wrap_count++;         /* 从低值跳到高值：正向过了一圈 */
  }
  last_raw = raw;
}

/* USER CODE END 0 */

/* 初始化 CAN 过滤器、启动 CAN、打开接收中断 */
void Motor_Init(void)
{
  CAN_FilterTypeDef sFilterConfig = {0};

  /* 过滤器：不过滤，接收所有标准帧（用于测试；CAN2 用 bank 14） */
  sFilterConfig.FilterIdHigh = 0x0000;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  sFilterConfig.FilterMaskIdLow = 0x0000;
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterBank = 14;              /* CAN2 的过滤器从 bank 14 开始 */
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterActivation = ENABLE;
  HAL_CAN_ConfigFilter(&hcan2, &sFilterConfig);

  HAL_CAN_Start(&hcan2);

  HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING);
}

/* 发送电流指令：CAN ID 0x200，8 字节 = 4 个电机各 2 字节（低字节在前） */
void Motor_SendCurrent(int16_t c1, int16_t c2, int16_t c3, int16_t c4)
{
  uint8_t data[8];
  data[0] = (uint8_t)(c1 & 0xFF);
  data[1] = (uint8_t)((c1 >> 8) & 0xFF);
  data[2] = (uint8_t)(c2 & 0xFF);
  data[3] = (uint8_t)((c2 >> 8) & 0xFF);
  data[4] = (uint8_t)(c3 & 0xFF);
  data[5] = (uint8_t)((c3 >> 8) & 0xFF);
  data[6] = (uint8_t)(c4 & 0xFF);
  data[7] = (uint8_t)((c4 >> 8) & 0xFF);

  CAN_TxHeaderTypeDef TxHeader = {0};
  TxHeader.StdId = 0x200;
  TxHeader.IDE = CAN_ID_STD;
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.DLC = 8;
  uint32_t TxMailbox;
  HAL_CAN_AddTxMessage(&hcan2, &TxHeader, data, &TxMailbox);
}

/* CAN 接收中断回调：解析 C620 反馈（ID 0x201~0x204） */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  if (hcan != &hcan2) {
    return;
  }

  CAN_RxHeaderTypeDef RxHeader = {0};
  uint8_t data[8];
  HAL_CAN_GetRxMessage(&hcan2, CAN_RX_FIFO0, &RxHeader, data);

  if (RxHeader.StdId >= 0x201 && RxHeader.StdId <= 0x204) {
    uint8_t idx = (uint8_t)(RxHeader.StdId - 0x201);
    motor[idx].speed_rpm      = (int16_t)(data[0] | ((uint16_t)data[1] << 8));
    motor[idx].angle_raw      = (uint16_t)(data[2] | ((uint16_t)data[3] << 8));
    motor[idx].torque_current = (int16_t)(data[4] | ((uint16_t)data[5] << 8));
    motor[idx].online         = 1U;
    if (idx == 0U) {
      Motor_UnwrapAngle(motor[0].angle_raw);
    }
  }
}

/* 读取电机0 输出轴角度(°)：多圈累计角度 ÷ 减速比 */
float Motor_GetAngleDeg(void)
{
  int32_t abs_raw = (int32_t)wrap_count * (int32_t)MOTOR_ENCODER_PER_REV
                    + (int32_t)last_raw;
  return (float)abs_raw * 360.0f / (float)MOTOR_ENCODER_PER_REV
         / MOTOR_GEAR_RATIO;
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
