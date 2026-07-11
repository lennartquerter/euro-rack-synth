/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#define JACK_DET_A_Pin GPIO_PIN_0
#define JACK_DET_A_GPIO_Port GPIOC
#define JACK_DET_B_Pin GPIO_PIN_1
#define JACK_DET_B_GPIO_Port GPIOC
#define CV_IN_A_Pin GPIO_PIN_2
#define CV_IN_A_GPIO_Port GPIOC
#define CV_IN_B_Pin GPIO_PIN_3
#define CV_IN_B_GPIO_Port GPIOC
#define ENC_SW_Pin GPIO_PIN_5
#define ENC_SW_GPIO_Port GPIOA
#define TRIG_A_IN_Pin GPIO_PIN_4
#define TRIG_A_IN_GPIO_Port GPIOC
#define TRIG_B_IN_Pin GPIO_PIN_5
#define TRIG_B_IN_GPIO_Port GPIOC
#define TRIG_A_OUT_Pin GPIO_PIN_0
#define TRIG_A_OUT_GPIO_Port GPIOB
#define TRIG_B_OUT_Pin GPIO_PIN_1
#define TRIG_B_OUT_GPIO_Port GPIOB
#define SW_LOAD_Pin GPIO_PIN_13
#define SW_LOAD_GPIO_Port GPIOB
#define BTN_A_Pin GPIO_PIN_6
#define BTN_A_GPIO_Port GPIOC
#define BTN_B_Pin GPIO_PIN_7
#define BTN_B_GPIO_Port GPIOC
#define BTN_C_Pin GPIO_PIN_8
#define BTN_C_GPIO_Port GPIOC
#define BTN_D_Pin GPIO_PIN_9
#define BTN_D_GPIO_Port GPIOC
#define LED_LATCH_Pin GPIO_PIN_11
#define LED_LATCH_GPIO_Port GPIOC
#define DAC_CS_Pin GPIO_PIN_4
#define DAC_CS_GPIO_Port GPIOB
#define MODE_Pin GPIO_PIN_2
#define MODE_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
