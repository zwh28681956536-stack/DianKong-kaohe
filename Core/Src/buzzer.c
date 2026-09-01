/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    buzzer.c
 * @brief   蜂鸣器驱动（PB8 / TIM4_CH3，PWM 驱动无源蜂鸣器）
 *
 * 时钟计算：
 *   TIM4 挂在 APB1 上，APB1 预分频 = 4 -> PCLK1 = 42MHz，定时器时钟 = 84MHz。
 *   当前 Prescaler = 83 -> 计数频率 = 84MHz / (83+1) = 1MHz。
 *   因此频率 f 对应的自动重装值 ARR = 1000000 / f，占空比 50%。
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "buzzer.h"
#include "cmsis_os.h"
#include "main.h"
#include "tim.h"

/* USER CODE BEGIN 0 */

/* --------------------------------------------------------------------------*/
/* 报错标志定义（供其他模块置位，defaultTask 轮询播放报错音调）              */
/* --------------------------------------------------------------------------*/
volatile uint8_t Buzzer_ErrFlag1 = 0U;
volatile uint8_t Buzzer_ErrFlag2 = 0U;

/* --------------------------------------------------------------------------*/
/* 配置区                                                                    */
/* --------------------------------------------------------------------------*/

/* 定时器计数频率：TIM4 时钟 84MHz / (Prescaler+1) = 1MHz */
#define BUZZER_TIM_CLK_HZ 1000000UL

/*
 * 车辆编号：用于比赛中区分不同车辆。
 * 修改此宏即可改变上电音调（0~3 各有不同的上电音调）。
 *   0 : C5 双短鸣（嘀-嘀）
 *   1 : C5 -> G5（上行二音）
 *   2 : C5 -> E5 -> G5（上行琶音 1-3-5）
 *   3 : G5 单长鸣
 */
#define BUZZER_VEHICLE_ID 0U

/* 音符数据结构：freq = 频率(Hz)，0 表示休止(静音)；ms = 持续时长 */
typedef struct
{
  uint16_t freq;
  uint16_t ms;
} BuzzerNote_t;

/* 上电提示音 —— 按车辆区分 */
static const BuzzerNote_t boot_melody_0[] = {
    {4000, 150},
    {0, 50},
    {4000, 150}, /* 4kHz 双短鸣（额定频率，最响） */
};
static const BuzzerNote_t boot_melody_1[] = {
    {4000, 150},
    {0, 50},
    {4500, 150}, /* 4kHz→4.5kHz：上行二音 */
};
static const BuzzerNote_t boot_melody_2[] = {
    {4000, 120},
    {0, 40},
    {4500, 120},
    {0, 40},
    {5000, 120}, /* 4k→4.5k→5k：上行琶音 */
};
static const BuzzerNote_t boot_melody_3[] = {
    {4000, 400}, /* 4kHz：单长鸣 */
};

/* 报错音调 1：小星星（Twinkle Twinkle Little Star） */
static const BuzzerNote_t err1_melody[] = {
    {523, 250},
    {523, 250},
    {784, 250},
    {784, 250},
    {880, 250},
    {880, 250},
    {784, 500},
    {698, 250},
    {698, 250},
    {659, 250},
    {659, 250},
    {587, 250},
    {587, 250},
    {523, 500},
};

/* 报错音调 2：欢乐颂（Ode to Joy） */
static const BuzzerNote_t err2_melody[] = {
    {659, 250},
    {659, 250},
    {698, 250},
    {784, 250},
    {784, 250},
    {698, 250},
    {659, 250},
    {587, 250},
    {523, 250},
    {523, 250},
    {587, 250},
    {659, 250},
    {659, 250},
    {587, 250},
    {587, 500},
};

#define BOOT_MELODY_0_LEN (sizeof(boot_melody_0) / sizeof(BuzzerNote_t))
#define BOOT_MELODY_1_LEN (sizeof(boot_melody_1) / sizeof(BuzzerNote_t))
#define BOOT_MELODY_2_LEN (sizeof(boot_melody_2) / sizeof(BuzzerNote_t))
#define BOOT_MELODY_3_LEN (sizeof(boot_melody_3) / sizeof(BuzzerNote_t))
#define ERR1_MELODY_LEN (sizeof(err1_melody) / sizeof(BuzzerNote_t))
#define ERR2_MELODY_LEN (sizeof(err2_melody) / sizeof(BuzzerNote_t))

/* --------------------------------------------------------------------------*/
/* 内部函数                                                                  */
/* --------------------------------------------------------------------------*/

static uint8_t buzzer_pwm_started = 0U;

/* 设置音符频率：freq = 0 则静音 */
static void Buzzer_SetNote(uint16_t freq)
{
  if (freq == 0U)
  {
    /* 静音：占空比置 0 */
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0U);
    return;
  }

  uint32_t arr = BUZZER_TIM_CLK_HZ / (uint32_t)freq;

  /* 保护：ARR 不能超出 16 位定时器范围 */
  if (arr > 0xFFFFU)
  {
    arr = 0xFFFFU;
  }
  if (arr == 0U)
  {
    arr = 1U;
  }

  /* 切换频率：计数器清零，重载 ARR 与 50% 占空比 */
  __HAL_TIM_SET_COUNTER(&htim4, 0U);
  __HAL_TIM_SET_AUTORELOAD(&htim4, arr);
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, arr / 2U);
}

/* 延时：调度器运行前用 HAL_Delay，运行后用 osDelay（不阻塞其它任务） */
static void Buzzer_Delay(uint32_t ms)
{
  if (osKernelGetState() == osKernelRunning)
  {
    osDelay(ms);
  }
  else
  {
    HAL_Delay(ms);
  }
}

/* 按曲谱播放整段旋律（阻塞延时），播完静音 */
static void Buzzer_PlayMelody(const BuzzerNote_t *melody, uint16_t len)
{
  uint16_t i;
  for (i = 0U; i < len; i++)
  {
    Buzzer_SetNote(melody[i].freq);
    Buzzer_Delay(melody[i].ms);
  }
  Buzzer_SetNote(0U);
}

/* USER CODE END 0 */

/* USER CODE BEGIN 1 */

/* 初始化 PWM 并保持静音 */
void Buzzer_Init(void)
{
  if (buzzer_pwm_started == 0U)
  {
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0U); /* 初始静音 */
    buzzer_pwm_started = 1U;
  }
}

/* 上电提示音：每次复位/上电播放一次，按车辆编号区分音调 */
void Buzzer_PlayBootBeep(void)
{
  switch (BUZZER_VEHICLE_ID)
  {
    case 1U:
      Buzzer_PlayMelody(boot_melody_1, BOOT_MELODY_1_LEN);
      break;
    case 2U:
      Buzzer_PlayMelody(boot_melody_2, BOOT_MELODY_2_LEN);
      break;
    case 3U:
      Buzzer_PlayMelody(boot_melody_3, BOOT_MELODY_3_LEN);
      break;
    case 0U:
    default:
      Buzzer_PlayMelody(boot_melody_0, BOOT_MELODY_0_LEN);
      break;
  }
}

/* 播放单个音符（阻塞延时） */
void Buzzer_PlayNote(uint16_t freq, uint16_t duration_ms)
{
  Buzzer_SetNote(freq);
  Buzzer_Delay(duration_ms);
  Buzzer_SetNote(0U);
}

/* 播放报错音调：errorType 选择两种歌曲之一（阻塞延时） */
void Buzzer_PlayErrorTone(Buzzer_ErrorType_t errorType)
{
  switch (errorType)
  {
    case BUZZER_ERR_1:
      Buzzer_PlayMelody(err1_melody, ERR1_MELODY_LEN);
      break;
    case BUZZER_ERR_2:
      Buzzer_PlayMelody(err2_melody, ERR2_MELODY_LEN);
      break;
    default:
      break;
  }
}

/* USER CODE END 1 */
