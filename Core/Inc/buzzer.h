/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    buzzer.h
  * @brief   蜂鸣器驱动（PB8 / TIM4_CH3，PWM 驱动无源蜂鸣器）
  *
  * 功能说明：
  *   1. 上电提示音：Buzzer_PlayBootBeep()，每次复位/上电播放一次，
  *      音调由 buzzer.c 中 BUZZER_VEHICLE_ID 决定，用于区分不同车辆。
  *   2. 两种报错音调：Buzzer_PlayErrorTone()，用两段不同的歌曲
  *      （小星星 / 欢乐颂）区分不同类型的报错。
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BUZZER_H
#define __BUZZER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* 报错类型：选择两种不同的报错音调（不同歌曲） */
typedef enum {
  BUZZER_ERR_NONE = 0,   /* 无报错 */
  BUZZER_ERR_1    = 1,   /* 报错音调 1：小星星 */
  BUZZER_ERR_2    = 2,   /* 报错音调 2：欢乐颂 */
} Buzzer_ErrorType_t;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/* USER CODE BEGIN Prototypes */

/* 报错标志：其他模块检测到故障时置 1，defaultTask 检查到后播放对应报错音调并清零 */
extern volatile uint8_t Buzzer_ErrFlag1;   /* 置 1 → 播放报错音调 1（小星星） */
extern volatile uint8_t Buzzer_ErrFlag2;   /* 置 1 → 播放报错音调 2（欢乐颂） */

/* 初始化 PWM 并保持静音（须在 MX_TIM4_Init() 之后调用一次） */
void Buzzer_Init(void);

/* 上电提示音：每次复位/上电播放一次，音调随 BUZZER_VEHICLE_ID 变化 */
void Buzzer_PlayBootBeep(void);

/* 播放单个音符（阻塞延时）：freq = 0 表示静音 */
void Buzzer_PlayNote(uint16_t freq, uint16_t duration_ms);

/* 播放报错音调（阻塞延时）：errorType 选择两种歌曲之一 */
void Buzzer_PlayErrorTone(Buzzer_ErrorType_t errorType);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /* __BUZZER_H */
