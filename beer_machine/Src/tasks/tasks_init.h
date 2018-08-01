#ifndef  __TASKS_H__
#define  __TASKS_H__
#include "stdint.h"


#ifdef  __cplusplus
#define TASKS_BEGIN  extern "C" {
#define TASKS_END    }
#else
#define TASKS_BEGIN  
#define TASKS_END   
#endif

TASKS_BEGIN

extern EventGroupHandle_t tasks_sync_evt_group_hdl;
void tasks_init();

#define  TASKS_SYNC_EVENT_ADC_TASK_RDY              (1<<0)
#define  TASKS_SYNC_EVENT_TEMPERATURE_TASK_RDY      (1<<1)
#define  TASKS_SYNC_EVENT_PRESSURE_TASK_RDY         (1<<2)
#define  TASKS_SYNC_EVENT_COMPRESSOR_TASK_RDY       (1<<3)
#define  TASKS_SYNC_EVENT_ALARM_TASK_RDY            (1<<4)
#define  TASKS_SYNC_EVENT_DISPLAY_TASK_RDY          (1<<5)
#define  TASKS_SYNC_EVENT_ALL_TASKS_RDY             ((1<<6)-1)





TASKS_END




#endif