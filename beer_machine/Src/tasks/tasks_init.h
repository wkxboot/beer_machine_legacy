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


typedef enum
{
T_ADC_COMPLETED=0,
P_ADC_COMPLETED,
REQ_PRESSURE_VALUE,
REQ_TEMPERATURE_VALUE,
REQ_CAPACITY_VALUE,
REQ_COMM_WIFI_STATUS,
REQ_COMM_4G_STATUS,
RESPONSE_TEMPERATURE_VALUE,
RESPONSE_PRESSURE_VALUE,
RESPONSE_CAPACITY_VALUE,
RESPONSE_COMM_WIFI_STATUS,
RESPONSE_COMM_4G_STATUS,
BROADCAST_TEMPERATURE_VALUE,
BROADCAST_PRESSURE_VALUE,
BROADCAST_CAPACITY_VALUE,
BROADCAST_COMM_WIFI_STATUS,
BROADCAST_COMM_4G_STATUS,
ALARM_SW_SHORT_PRESS,
ALARM_SW_LONG_PRESS,
DISPLAY_FLASH_TIMER,
COMPRESSOR_START,
COMPRESSOR_STOP
}task_msg_type_t;


typedef struct
{
task_msg_type_t type;
union
{
uint8_t        pressure;
uint8_t        capacity;
uint8_t        comm_wifi;
uint8_t        comm_4g;
int16_t        temperature;
uint16_t       adc;
uint32_t       reserved;
};
osMessageQId   req_q_id;
}task_msg_t;



#ifndef  NULL
#define  NULL          (void*)0
#endif


#ifndef  TRUE       
#define  TRUE         1
#endif

#ifndef  FALSE       
#define  FALSE        0
#endif








TASKS_END




#endif