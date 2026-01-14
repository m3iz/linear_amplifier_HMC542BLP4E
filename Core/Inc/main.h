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
#include "stm32f3xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include "hmc542blp4e.h"
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
#define SHIFT_CLK_Pin GPIO_PIN_13
#define SHIFT_CLK_GPIO_Port GPIOC
#define RST_Pin GPIO_PIN_14
#define RST_GPIO_Port GPIOC
#define SHDN_Pin GPIO_PIN_4
#define SHDN_GPIO_Port GPIOA
#define LE_Pin GPIO_PIN_7
#define LE_GPIO_Port GPIOA
#define LED3_Pin GPIO_PIN_0
#define LED3_GPIO_Port GPIOB
#define RST2_Pin GPIO_PIN_14
#define RST2_GPIO_Port GPIOB
#define LDAC_Pin GPIO_PIN_15
#define LDAC_GPIO_Port GPIOB
#define SHIFT_CLK2_Pin GPIO_PIN_8
#define SHIFT_CLK2_GPIO_Port GPIOA
#define RSII_Q2_EX_Pin GPIO_PIN_11
#define RSII_Q2_EX_GPIO_Port GPIOA
#define RSII_Q1_EX_Pin GPIO_PIN_12
#define RSII_Q1_EX_GPIO_Port GPIOA
#define Chip_Pin GPIO_PIN_15
#define Chip_GPIO_Port GPIOA
#define DATA2_Pin GPIO_PIN_6
#define DATA2_GPIO_Port GPIOB
#define LE2_Pin GPIO_PIN_7
#define LE2_GPIO_Port GPIOB
#define DATA_Pin GPIO_PIN_9
#define DATA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
