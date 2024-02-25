/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.h
  * @brief   This file contains all the function prototypes for
  *          the gpio.c file
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
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
// Define macros for each pin
//#define BATT_On  CTRL_BATT_GPIO_Port->BSRR |=  CTRL_BATT_Pin // Turn on CTRL_BATT pin
//#define BATT_Off CTRL_BATT_GPIO_Port->BSRR &= ~CTRL_BATT_Pin // Turn off CTRL_BATT pin

//#define SEN1_On  GPIOA->BSRR |=  SEN1_Pin // Turn on SEN1 pin 0
//#define SEN1_Off GPIOA->BSRR &= ~SEN1_Pin // Turn off SEN1 pin

//#define SEN2_On  GPIOA->BSRR |=  SEN2_Pin // Turn on SEN2 pin 1
//#define SEN2_Off GPIOA->BSRR &= ~SEN2_Pin // Turn off SEN2 pin

//#define SPI1_RELEASE  SPI1_CS_GPIO_Port->BSRR |=  SPI1_CS_Pin // Turn on SPI1_CS pin
//#define SPI1_SELECT		SPI1_CS_GPIO_Port->BSRR &= ~SPI1_CS_Pin // Turn off SPI1_CS pin

//#define CTRL_SEN_On  CTRL_SEN_GPIO_Port->BSRR |=  CTRL_SEN_Pin // Turn on CTRL_SEN pin
//#define CTRL_SEN_Off CTRL_SEN_GPIO_Port->BSRR &= ~CTRL_SEN_Pin // Turn off CTRL_SEN pin

//#define Shutter2_On  GPIOC->BSRR |=  S2_Pin // Turn on S2 pin 6 64
//#define Shutter2_Off GPIOC->BSRR &= ~S2_Pin // Turn off S2 pin

//#define Focus2_On  GPIOC->BSRR |=  F2_Pin // Turn on F2 pin
//#define Focus2_Off GPIOC->BSRR &= ~F2_Pin // Turn off F2 pin

//#define Shutter1_On  GPIOC->BSRR |=  S1_Pin // Turn on S1 pin 8 256
//#define Shutter1_Off GPIOC->BSRR &= ~S1_Pin // Turn off S1 pin

//#define Focus1_On  GPIOC->BSRR |=  F1_Pin // Turn on F1 pin
//#define Focus1_Off GPIOC->BSRR &= ~F1_Pin // Turn off F1 pin
/////////////////////////////////////////////////////////////////////////
#define Valve_On(x)    SEN1_GPIO_Port->BSRR |=  (uint16_t)(x + 1) //Power ON
#define Valve_Off(x)   SEN1_GPIO_Port->BSRR &= ~(uint16_t)(x + 1) //Power OFF


#define OFF	GPIO_PIN_RESET
#define	ON GPIO_PIN_SET


/* USER CODE END Private defines */

void MX_GPIO_Init(void);

/* USER CODE BEGIN Prototypes */
void setFocus1(const GPIO_PinState state);
void setFocus2(const GPIO_PinState state);
void setShutter1(const GPIO_PinState state);
void setShutter2(const GPIO_PinState state);
void setBatteryCtrl(const GPIO_PinState state);
void setSpiSelect(const GPIO_PinState state);
void setAdPower(const GPIO_PinState state);

void setPinsValveMode(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ GPIO_H__ */

