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
#include "stm32g4xx_hal.h"

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
#define SIZE_Pin GPIO_PIN_0
#define SIZE_GPIO_Port GPIOC
#define SIZE_CV_Pin GPIO_PIN_1
#define SIZE_CV_GPIO_Port GPIOC
#define V_OCT_Pin GPIO_PIN_0
#define V_OCT_GPIO_Port GPIOA
#define POSITION_Pin GPIO_PIN_1
#define POSITION_GPIO_Port GPIOA
#define DENSITY_Pin GPIO_PIN_2
#define DENSITY_GPIO_Port GPIOA
#define PITCH_Pin GPIO_PIN_3
#define PITCH_GPIO_Port GPIOA
#define TEXTURE_Pin GPIO_PIN_6
#define TEXTURE_GPIO_Port GPIOA
#define TEXTURE_CV_Pin GPIO_PIN_7
#define TEXTURE_CV_GPIO_Port GPIOA
#define BLEND_Pin GPIO_PIN_4
#define BLEND_GPIO_Port GPIOC
#define BLEND_CV_Pin GPIO_PIN_14
#define BLEND_CV_GPIO_Port GPIOB
#define SPI3_ENABLE_Pin GPIO_PIN_11
#define SPI3_ENABLE_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
