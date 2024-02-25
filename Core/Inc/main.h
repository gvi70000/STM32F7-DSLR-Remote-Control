/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "stm32f7xx_hal.h"

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
#define CTRL_BATT_Pin GPIO_PIN_13
#define CTRL_BATT_GPIO_Port GPIOC
#define READ_BATT_Pin GPIO_PIN_0
#define READ_BATT_GPIO_Port GPIOC
#define SEN1_Pin GPIO_PIN_0
#define SEN1_GPIO_Port GPIOA
#define SEN2_Pin GPIO_PIN_1
#define SEN2_GPIO_Port GPIOA
#define INT_R_Pin GPIO_PIN_3
#define INT_R_GPIO_Port GPIOA
#define INT_R_EXTI_IRQn EXTI3_IRQn
#define SPI1_CS_Pin GPIO_PIN_4
#define SPI1_CS_GPIO_Port GPIOA
#define READ_SEN_Pin GPIO_PIN_4
#define READ_SEN_GPIO_Port GPIOC
#define CTRL_SEN_Pin GPIO_PIN_0
#define CTRL_SEN_GPIO_Port GPIOB
#define BUZZ_Pin GPIO_PIN_13
#define BUZZ_GPIO_Port GPIOB
#define S2_Pin GPIO_PIN_6
#define S2_GPIO_Port GPIOC
#define F2_Pin GPIO_PIN_7
#define F2_GPIO_Port GPIOC
#define S1_Pin GPIO_PIN_8
#define S1_GPIO_Port GPIOC
#define F1_Pin GPIO_PIN_9
#define F1_GPIO_Port GPIOC
#define ESP_RST_Pin GPIO_PIN_15
#define ESP_RST_GPIO_Port GPIOA
#define ESP_RX_Pin GPIO_PIN_10
#define ESP_RX_GPIO_Port GPIOC
#define ESP_TX_Pin GPIO_PIN_11
#define ESP_TX_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
