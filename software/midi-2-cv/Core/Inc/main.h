/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32f4xx_hal.h"

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
#define GATE_A_CONNECTED_Pin GPIO_PIN_0
#define GATE_A_CONNECTED_GPIO_Port GPIOA
#define GATE_B_CONNECTED_Pin GPIO_PIN_1
#define GATE_B_CONNECTED_GPIO_Port GPIOA
#define GATE_C_CONNECTED_Pin GPIO_PIN_2
#define GATE_C_CONNECTED_GPIO_Port GPIOA
#define GATE_D_CONNECTED_Pin GPIO_PIN_3
#define GATE_D_CONNECTED_GPIO_Port GPIOA
#define MIDI_CLOCK_OUT_Pin GPIO_PIN_7
#define MIDI_CLOCK_OUT_GPIO_Port GPIOA
#define GATE_A_Pin GPIO_PIN_12
#define GATE_A_GPIO_Port GPIOB
#define GATE_B_Pin GPIO_PIN_13
#define GATE_B_GPIO_Port GPIOB
#define GATE_C_Pin GPIO_PIN_14
#define GATE_C_GPIO_Port GPIOB
#define GATE_D_Pin GPIO_PIN_15
#define GATE_D_GPIO_Port GPIOB
#define MIDI_CLOCK_OUT_PWM_Pin GPIO_PIN_9
#define MIDI_CLOCK_OUT_PWM_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
