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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_RUN_MODE_Pin GPIO_PIN_13
#define LED_RUN_MODE_GPIO_Port GPIOC
#define LED_STOP_MODE_Pin GPIO_PIN_14
#define LED_STOP_MODE_GPIO_Port GPIOC
#define LED_EDIT_MODE_Pin GPIO_PIN_15
#define LED_EDIT_MODE_GPIO_Port GPIOC
#define CLK_INPUT_Pin GPIO_PIN_1
#define CLK_INPUT_GPIO_Port GPIOA
#define DAC_ENABLE_Pin GPIO_PIN_4
#define DAC_ENABLE_GPIO_Port GPIOA
#define GATE_B_OUT_Pin GPIO_PIN_14
#define GATE_B_OUT_GPIO_Port GPIOB
#define GATE_A_OUT_Pin GPIO_PIN_15
#define GATE_A_OUT_GPIO_Port GPIOB
#define ENCODER_BTN_Pin GPIO_PIN_10
#define ENCODER_BTN_GPIO_Port GPIOA
#define ENCODER_BTN_EXTI_IRQn EXTI15_10_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
