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
#define DCDC_PG_Pin GPIO_PIN_2
#define DCDC_PG_GPIO_Port GPIOE
#define ENCODER_KEY_Pin GPIO_PIN_4
#define ENCODER_KEY_GPIO_Port GPIOC
#define UI_START_Pin GPIO_PIN_9
#define UI_START_GPIO_Port GPIOE
#define UI_BACK_Pin GPIO_PIN_10
#define UI_BACK_GPIO_Port GPIOE
#define SOFT_STOP_Pin GPIO_PIN_11
#define SOFT_STOP_GPIO_Port GPIOE
#define SOFT_STOP_EXTI_IRQn EXTI15_10_IRQn
#define HX711_DOUT_Pin GPIO_PIN_8
#define HX711_DOUT_GPIO_Port GPIOD
#define HX711_DOUT_EXTI_IRQn EXTI9_5_IRQn
#define HX711_SCK_Pin GPIO_PIN_9
#define HX711_SCK_GPIO_Port GPIOD
#define TF_DETECT_Pin GPIO_PIN_7
#define TF_DETECT_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
