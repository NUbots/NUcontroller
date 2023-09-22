/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "stm32h7xx_hal.h"

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
#define DXL_DIR4_Pin GPIO_PIN_2
#define DXL_DIR4_GPIO_Port GPIOA
#define MPU_INT_Pin GPIO_PIN_10
#define MPU_INT_GPIO_Port GPIOE
#define MPU_INT_EXTI_IRQn EXTI15_10_IRQn
#define MPU_NSS_Pin GPIO_PIN_11
#define MPU_NSS_GPIO_Port GPIOE
#define MPU_SCK_Pin GPIO_PIN_12
#define MPU_SCK_GPIO_Port GPIOE
#define MPU_MISO_Pin GPIO_PIN_13
#define MPU_MISO_GPIO_Port GPIOE
#define MPU_MOSI_Pin GPIO_PIN_14
#define MPU_MOSI_GPIO_Port GPIOE
#define DXL_DIR3_Pin GPIO_PIN_10
#define DXL_DIR3_GPIO_Port GPIOD
#define DXL_DIR1_Pin GPIO_PIN_11
#define DXL_DIR1_GPIO_Port GPIOD
#define DXL_DIR6_Pin GPIO_PIN_15
#define DXL_DIR6_GPIO_Port GPIOD
#define SPARE1_Pin GPIO_PIN_0
#define SPARE1_GPIO_Port GPIOD
#define DXL_DIR5_Pin GPIO_PIN_1
#define DXL_DIR5_GPIO_Port GPIOD
#define SPARE2_Pin GPIO_PIN_3
#define SPARE2_GPIO_Port GPIOD
#define DXL_DIR2_Pin GPIO_PIN_4
#define DXL_DIR2_GPIO_Port GPIOD
#define DXL_PWR_EN_Pin GPIO_PIN_6
#define DXL_PWR_EN_GPIO_Port GPIOB
#define BUZZER_SIG_Pin GPIO_PIN_7
#define BUZZER_SIG_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
