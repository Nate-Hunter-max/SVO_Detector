/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define V_AMP_Pin GPIO_PIN_0
#define V_AMP_GPIO_Port GPIOA
#define BTN_P_Pin GPIO_PIN_1
#define BTN_P_GPIO_Port GPIOA
#define BTN_M_Pin GPIO_PIN_2
#define BTN_M_GPIO_Port GPIOA
#define CTRL_UP_Pin GPIO_PIN_3
#define CTRL_UP_GPIO_Port GPIOA
#define CTRL_DWN_Pin GPIO_PIN_4
#define CTRL_DWN_GPIO_Port GPIOA
#define SWITCH_Pin GPIO_PIN_5
#define SWITCH_GPIO_Port GPIOA
#define BUZZER_Pin GPIO_PIN_7
#define BUZZER_GPIO_Port GPIOA
#define LED_Pin GPIO_PIN_9
#define LED_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
