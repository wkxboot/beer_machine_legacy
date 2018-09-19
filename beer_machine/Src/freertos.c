/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */     
#include "iwdg.h"
#include "i2c.h"
#include "cpu_utils.h"
#include "tasks_init.h"
#include "adc_task.h"
#include "alarm_task.h"
#include "compressor_task.h"
#include "display_task.h"
#include "temperature_task.h"
#include "pressure_task.h"
#include "led.h"
#include "firmware_version.h"
#include "log.h"
#define  LOG_MODULE_LEVEL    LOG_LEVEL_DEBUG
#define  LOG_MODULE_NAME     "[main]"
/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle;

/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* Pre/Post sleep processing prototypes */
void PreSleepProcessing(uint32_t *ulExpectedIdleTime);
void PostSleepProcessing(uint32_t *ulExpectedIdleTime);

/* Hook prototypes */
void vApplicationIdleHook(void);
void vApplicationTickHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 2 */
__weak void vApplicationIdleHook( void )
{
   /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
   to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
   task. It is essential that code added to this hook function never attempts
   to block in any way (for example, call xQueueReceive() with a block time
   specified, or call vTaskDelay()). If the application makes use of the
   vTaskDelete() API function (as this demo application does) then it is also
   important that vApplicationIdleHook() is permitted to return to its calling
   function, because it is the responsibility of the idle task to clean up
   memory allocated by the kernel to any task that has since been deleted. */
}
/* USER CODE END 2 */

/* USER CODE BEGIN 3 */
__weak void vApplicationTickHook( void )
{
   /* This function will be called by each tick interrupt if
   configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h. User code can be
   added here, but the tick hook is called from an interrupt context, so
   code must not attempt to block, and only the interrupt safe FreeRTOS API
   functions can be used (those that end in FromISR()). */
}
/* USER CODE END 3 */

/* USER CODE BEGIN 4 */
__weak void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
   log_assert(0);
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
__weak void vApplicationMallocFailedHook(void)
{
   /* vApplicationMallocFailedHook() will only be called if
   configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
   function that will get called if a call to pvPortMalloc() fails.
   pvPortMalloc() is called internally by the kernel whenever a task, queue,
   timer or semaphore is created. It is also called by various parts of the
   demo application. If heap_1.c or heap_2.c are used, then the size of the
   heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
   FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
   to query the size of free heap space that remains (although it does not
   provide information on how the remaining heap might be fragmented). */
  log_assert(0);
}
/* USER CODE END 5 */

/* USER CODE BEGIN PREPOSTSLEEP */
__weak void PreSleepProcessing(uint32_t *ulExpectedIdleTime)
{
/* place for user code */ 
}

__weak void PostSleepProcessing(uint32_t *ulExpectedIdleTime)
{
/* place for user code */
}
/* USER CODE END PREPOSTSLEEP */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
    log_init();
    tasks_init();    
    log_debug("version:%s.\r\n",FIRMWARE_VERSION_STR);
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  
  osThreadDef(adc_task,adc_task, osPriorityNormal, 0, 128);
  adc_task_hdl = osThreadCreate(osThread(adc_task), NULL);
    /* temperature_task */
  osThreadDef(temperature_task, temperature_task, osPriorityNormal, 0, 128);
  temperature_task_hdl = osThreadCreate(osThread(temperature_task), NULL);
  
    /* pressure_task */
  osThreadDef(pressure_task, pressure_task, osPriorityNormal, 0, 128);
  pressure_task_hdl = osThreadCreate(osThread(pressure_task), NULL);
  
    /* compressor_task */
  osThreadDef(compressor_task, compressor_task, osPriorityNormal, 0, 128);
  compressor_task_hdl = osThreadCreate(osThread(compressor_task), NULL);
  
    /* alarm_task */
  osThreadDef(alarm_task, alarm_task, osPriorityNormal, 0, 144);
  alarm_task_hdl = osThreadCreate(osThread(alarm_task), NULL);
  
    /* display_task */
  osThreadDef(display_task, display_task, osPriorityNormal, 0, 128);
  display_task_hdl = osThreadCreate(osThread(display_task), NULL);
  
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
}

/* StartDefaultTask function */
void StartDefaultTask(void const * argument)
{

  /* USER CODE BEGIN StartDefaultTask */
  uint8_t debug[3];
  uint8_t w_eeprom[16],r_eeprom[16];
  
  HAL_StatusTypeDef status;
  for(uint8_t i=0;i<16;i++){
  w_eeprom[i]=i;
  }
  
  for(;;)
  {
    osDelay(200);
    sys_feed_dog();
    if(log_read(debug,3)==3){
      switch(debug[0]){
      case 'u':
      log_warning("cpu:%d%%.\r\n",osGetCPUUsage());
      break;
      
      case 'w':
      status = HAL_I2C_Mem_Write(&hi2c1,0xa0,0x00,I2C_MEMADD_SIZE_8BIT,w_eeprom,16,0xfff);
      if(status !=HAL_OK){
      log_error("write eeprom error.%d.\r\n",status);
      }
      log_debug("write eeprom ok.%d.\r\n",status);
      break;
     
      case 'r':
      status = HAL_I2C_Mem_Read(&hi2c1,0xa0,0x00,I2C_MEMADD_SIZE_8BIT,r_eeprom,16,0xfff);
      if(status !=HAL_OK){
      log_error("read eeprom error.%d.\r\n",status);
      }
      for(uint8_t i=0;i<16;i++){
      log_debug("r[%d]=%d.\r\n",i,r_eeprom[i]);
      }
      break;
      case 'b':
      led_display_brightness(debug[1]-'0');
      log_debug("brightness:%d.\r\n",debug[1]-'0');
      break;
      default:
      break;
      }
     }
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Application */
uint32_t log_time()
{
 return osKernelSysTick();
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
