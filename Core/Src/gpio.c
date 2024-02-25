/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, CTRL_BATT_Pin|S2_Pin|F2_Pin|S1_Pin
                          |F1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, SPI1_CS_Pin|ESP_RST_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CTRL_SEN_GPIO_Port, CTRL_SEN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PCPin PCPin PCPin PCPin
                           PCPin */
  GPIO_InitStruct.Pin = CTRL_BATT_Pin|S2_Pin|F2_Pin|S1_Pin
                          |F1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = INT_R_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(INT_R_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PAPin PAPin */
  GPIO_InitStruct.Pin = SPI1_CS_Pin|ESP_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = CTRL_SEN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CTRL_SEN_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

}

/* USER CODE BEGIN 2 */

void setFocus1(const GPIO_PinState state) {
	HAL_GPIO_WritePin(F1_GPIO_Port, F1_Pin, state);
	//state ? (F1_GPIO_Port->BSRR |= F1_Pin) : (F1_GPIO_Port->BSRR &= ~F1_Pin);
}

void setFocus2(const GPIO_PinState state) {
	HAL_GPIO_WritePin(F2_GPIO_Port, F2_Pin, state);
	//state ? (F2_GPIO_Port->BSRR |= F2_Pin) : (F2_GPIO_Port->BSRR &= ~F2_Pin);
}

void setShutter1(const GPIO_PinState state) {
	HAL_GPIO_WritePin(S1_GPIO_Port, S1_Pin, state);
	//state ? (S1_GPIO_Port->BSRR |= S1_Pin) : (S1_GPIO_Port->BSRR &= ~S1_Pin);
}

void setShutter2(const GPIO_PinState state) {
	HAL_GPIO_WritePin(S2_GPIO_Port, S2_Pin, state);
	//state ? (S2_GPIO_Port->BSRR |= S2_Pin) : (S2_GPIO_Port->BSRR &= ~S2_Pin);
}

void setBatteryCtrl(const GPIO_PinState state) {
	HAL_GPIO_WritePin(CTRL_BATT_GPIO_Port, CTRL_BATT_Pin, state);
	//state ? (CTRL_BATT_GPIO_Port->BSRR |= CTRL_BATT_Pin) : (CTRL_BATT_GPIO_Port->BSRR &= ~CTRL_BATT_Pin);
}

void setSpiSelect(const GPIO_PinState state) {
	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, state);
	//state ? (SPI1_CS_GPIO_Port->BSRR |= SPI1_CS_Pin) : (SPI1_CS_GPIO_Port->BSRR &= ~SPI1_CS_Pin);
}

void setAdPower(const GPIO_PinState state) {
	HAL_GPIO_WritePin(CTRL_SEN_GPIO_Port, CTRL_SEN_Pin, state);
	//state ? (CTRL_SEN_GPIO_Port->BSRR |= CTRL_SEN_Pin) : (CTRL_SEN_GPIO_Port->BSRR &= ~CTRL_SEN_Pin);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == INT_R_Pin)
  {
    //HAL_GPIO_TogglePin(F1_GPIO_Port, F1_Pin);
  } 
}

//Set the pins as GPIO Output to control the valves
void setPinsValveMode(void){ 
	//   __HAL_RCC_GPIOA_CLK_ENABLE(); // The clock is already set up in MX_GPIO_Init
	GPIO_InitTypeDef GPIO_InitStruct = {0};
  HAL_GPIO_WritePin(SEN1_GPIO_Port, SEN1_Pin|SEN2_Pin, GPIO_PIN_RESET); // Set pins LOW
  GPIO_InitStruct.Pin = SEN2_Pin|SEN1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);  
}

/* USER CODE END 2 */
