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
#define TRIG_2_IN_Pin GPIO_PIN_15
#define TRIG_2_IN_GPIO_Port GPIOC
#define TRIG_1_EN_Pin GPIO_PIN_0
#define TRIG_1_EN_GPIO_Port GPIOC
#define TRIG_2_EN_Pin GPIO_PIN_1
#define TRIG_2_EN_GPIO_Port GPIOC
#define ENC_SW_Pin GPIO_PIN_5
#define ENC_SW_GPIO_Port GPIOA
#define LED_1_Pin GPIO_PIN_12
#define LED_1_GPIO_Port GPIOB
#define LED_SW_1_Pin GPIO_PIN_13
#define LED_SW_1_GPIO_Port GPIOB
#define LED_SW_2_Pin GPIO_PIN_15
#define LED_SW_2_GPIO_Port GPIOB
#define CHAR_1_Pin GPIO_PIN_11
#define CHAR_1_GPIO_Port GPIOA
#define CHAR_2_Pin GPIO_PIN_12
#define CHAR_2_GPIO_Port GPIOA
#define DISP_ENABLE_Pin GPIO_PIN_11
#define DISP_ENABLE_GPIO_Port GPIOC
#define SPI1_SYNC_Pin GPIO_PIN_4
#define SPI1_SYNC_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
