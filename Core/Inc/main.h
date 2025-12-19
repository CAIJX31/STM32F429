/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
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
#include "stdio.h"
#include "sys.h"
#include "usart.h"
#include "tim.h"
#include "gpio.h"
#include "arm_math.h"
#define ADC_BUF_LEN 4096U
extern uint16_t adc_buf[ADC_BUF_LEN];
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#define COMM_INFO 3
#define WARN_INFO 2
#define ERRO_INFO 1
#define DEBUG_CLASS 4

#if DEBUG_CLASS == ERRO_INFO
#define DEBUG_INFO(format,...) my_printf(format,##__VA_ARGS__)  
#define DEBUG_WARN(format,...) my_printf(format,##__VA_ARGS__)  
#define DEBUG_ERRO(format,...) my_printf(format,##__VA_ARGS__)  
#elif DEBUG_CLASS == WARN_INFO
#define DEBUG_INFO(format,...) my_printf(format,##__VA_ARGS__)   
#define DEBUG_WARN(format,...) my_printf(format,##__VA_ARGS__)  
#define DEBUG_ERRO(format,...) 
#elif DEBUG_CLASS == COMM_INFO
#define DEBUG_INFO(format,...) my_printf(format,##__VA_ARGS__)  
#define DEBUG_WARN(format,...)  
#define DEBUG_ERRO(format,...) 
#else
#define DEBUG_INFO(format,...)  
#define DEBUG_WARN(format,...)  
#define DEBUG_ERRO(format,...)   
#endif  
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
  typedef struct
  {
      uint8_t u_p : 1;
      uint8_t u_n : 1;
      uint8_t v_p : 1;
      uint8_t v_n : 1;
      uint8_t w_p : 1;
      uint8_t w_n : 1;
      uint8_t reserve : 2;
  } exception_flagbit_t; 
  
  
typedef union 
  {
      exception_flagbit_t bit;
      uint8_t byte;
  }exception_flag_t;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
  // 2022-2-6
  /*
    uvw_btn_flag:
    0  init value
    1  u- is clicked and testing
    2  v- is clicked and testing
    3  w- is clicked and testing
    4  u- is clicked and runover
    5  v- is clicked and runover
    6  w- is clicked and runover
  */
  extern int uvw_btn_flag;

  extern uint32_t times;
//  extern uint8_t exception_flag;
  

  extern exception_flag_t exception_flag;
  extern uint8_t fatWorkingBuf[4096];
  extern char fsPath[256];
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SIGNAL_SPWM_EN_Pin GPIO_PIN_2
#define SIGNAL_SPWM_EN_GPIO_Port GPIOE
#define LOGIC_CTRL_IN1_Pin GPIO_PIN_3
#define LOGIC_CTRL_IN1_GPIO_Port GPIOE
#define LOGIC_CTRL_IN2_Pin GPIO_PIN_4
#define LOGIC_CTRL_IN2_GPIO_Port GPIOE
#define I_U1_Pin GPIO_PIN_2
#define I_U1_GPIO_Port GPIOD
#define I_U2_Pin GPIO_PIN_3
#define I_U2_GPIO_Port GPIOD
#define I_V1_Pin GPIO_PIN_4
#define I_V1_GPIO_Port GPIOD
#define I_V2_Pin GPIO_PIN_5
#define I_V2_GPIO_Port GPIOD
#define I_W1_Pin GPIO_PIN_6
#define I_W1_GPIO_Port GPIOD
#define I_W2_Pin GPIO_PIN_7
#define I_W2_GPIO_Port GPIOD
#define LCD_BL_Pin GPIO_PIN_5
#define LCD_BL_GPIO_Port GPIOB
#define SIGNAL_EN_24A_Pin GPIO_PIN_6
#define SIGNAL_EN_24A_GPIO_Port GPIOB
#define SIGNAL_EN_24B_Pin GPIO_PIN_7
#define SIGNAL_EN_24B_GPIO_Port GPIOB
#define SIGNAL_EN_24C_Pin GPIO_PIN_8
#define SIGNAL_EN_24C_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
