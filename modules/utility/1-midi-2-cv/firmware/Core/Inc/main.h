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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SW_MODE_1_Pin GPIO_PIN_0
#define SW_MODE_1_GPIO_Port GPIOB
#define SW_MODE_1_EXTI_IRQn EXTI0_IRQn
#define SW_MODE_2_Pin GPIO_PIN_1
#define SW_MODE_2_GPIO_Port GPIOB
#define SW_MODE_2_EXTI_IRQn EXTI1_IRQn
#define GATE_1_OUT_Pin GPIO_PIN_12
#define GATE_1_OUT_GPIO_Port GPIOB
#define GATE_2_OUT_Pin GPIO_PIN_13
#define GATE_2_OUT_GPIO_Port GPIOB
#define GATE_3_OUT_Pin GPIO_PIN_14
#define GATE_3_OUT_GPIO_Port GPIOB
#define GATE_4_OUT_Pin GPIO_PIN_15
#define GATE_4_OUT_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

// Jack-detect (normalling) contacts on the gate outputs. Each output jack's tip-normal pin
// sits on a 100K pull-up to +3.3V and shorts to the tip while no plug is inserted, so the
// line follows the gate output when the jack is empty and floats high once a plug lifts the
// contact. Named to match GATE_n_CALLBACK on the schematic.
#define GATE_1_CALLBACK_Pin GPIO_PIN_1
#define GATE_1_CALLBACK_GPIO_Port GPIOA
#define GATE_2_CALLBACK_Pin GPIO_PIN_2
#define GATE_2_CALLBACK_GPIO_Port GPIOA
#define GATE_3_CALLBACK_Pin GPIO_PIN_4
#define GATE_3_CALLBACK_GPIO_Port GPIOA
#define GATE_4_CALLBACK_Pin GPIO_PIN_5
#define GATE_4_CALLBACK_GPIO_Port GPIOA

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
