/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__

/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define TX_4G_Pin GPIO_PIN_2
#define TX_4G_GPIO_Port GPIOA
#define RX_4G_Pin GPIO_PIN_3
#define RX_4G_GPIO_Port GPIOA
#define IO_4G_CTRL_Pin GPIO_PIN_4
#define IO_4G_CTRL_GPIO_Port GPIOA
#define PRESSURE_AD_Pin GPIO_PIN_5
#define PRESSURE_AD_GPIO_Port GPIOA
#define ALARM_CLEAR_SW_Pin GPIO_PIN_6
#define ALARM_CLEAR_SW_GPIO_Port GPIOA
#define ALARM_CLEAR_SW_EXTI_IRQn EXTI9_5_IRQn
#define COMPRESSOR_CTRL_Pin GPIO_PIN_7
#define COMPRESSOR_CTRL_GPIO_Port GPIOA
#define YW_AD_Pin GPIO_PIN_0
#define YW_AD_GPIO_Port GPIOB
#define TEMPERATURE_AD_Pin GPIO_PIN_1
#define TEMPERATURE_AD_GPIO_Port GPIOB
#define BUZZER_CTRL_Pin GPIO_PIN_12
#define BUZZER_CTRL_GPIO_Port GPIOB
#define TM1629A_SCK_Pin GPIO_PIN_13
#define TM1629A_SCK_GPIO_Port GPIOB
#define TM1629A_DIN_Pin GPIO_PIN_15
#define TM1629A_DIN_GPIO_Port GPIOB
#define TM1629A_CS_Pin GPIO_PIN_8
#define TM1629A_CS_GPIO_Port GPIOA
#define TX_WIFI_Pin GPIO_PIN_9
#define TX_WIFI_GPIO_Port GPIOA
#define RX_WIFI_Pin GPIO_PIN_10
#define RX_WIFI_GPIO_Port GPIOA
#define EEPROM_WP_CTRL_Pin GPIO_PIN_5
#define EEPROM_WP_CTRL_GPIO_Port GPIOB
#define EEPROM_SCL_Pin GPIO_PIN_6
#define EEPROM_SCL_GPIO_Port GPIOB
#define EEPROM_SDA_Pin GPIO_PIN_7
#define EEPROM_SDA_GPIO_Port GPIOB

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
 #define USE_FULL_ASSERT    1U 

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
