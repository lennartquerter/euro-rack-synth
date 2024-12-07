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
#define GATE_A_OUT_Pin GPIO_PIN_0
#define GATE_A_OUT_GPIO_Port GPIOC
#define GATE_B_OUT_Pin GPIO_PIN_1
#define GATE_B_OUT_GPIO_Port GPIOC
#define SPI4_CS_Pin GPIO_PIN_2
#define SPI4_CS_GPIO_Port GPIOA
#define POT_A_Pin GPIO_PIN_4
#define POT_A_GPIO_Port GPIOA
#define POT_B_Pin GPIO_PIN_5
#define POT_B_GPIO_Port GPIOA
#define TIM3_BTN_Pin GPIO_PIN_4
#define TIM3_BTN_GPIO_Port GPIOC
#define CLK_IN_Pin GPIO_PIN_9
#define CLK_IN_GPIO_Port GPIOC
#define BTN_1_Pin GPIO_PIN_9
#define BTN_1_GPIO_Port GPIOA
#define BTN_2_Pin GPIO_PIN_10
#define BTN_2_GPIO_Port GPIOA
#define BTN_3_Pin GPIO_PIN_11
#define BTN_3_GPIO_Port GPIOA
#define SPI1_CS_Pin GPIO_PIN_4
#define SPI1_CS_GPIO_Port GPIOB
#define TIM4_BTN_Pin GPIO_PIN_8
#define TIM4_BTN_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
