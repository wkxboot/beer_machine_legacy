#include "cmsis_os.h"
#include "tasks_init.h"
#include "adc_task.h"
#include "alarm_task.h"
#include "pressure_task.h"
#include "display_task.h"
#include "log.h"
#define  LOG_MODULE_LEVEL    LOG_LEVEL_DEBUG
#define  LOG_MODULE_NAME     "[p_task]"

osThreadId   pressure_task_hdl;
osMessageQId pressure_task_msg_q_id;

static task_msg_t   *ptr_msg;
static task_msg_t   d_msg;
static task_msg_t   a_msg;
static task_msg_t   response_msg;

typedef enum
{
P_DIR_INIT=0,
P_DIR_UP,
P_DIR_DOWN
}pressure_dir_t;

typedef struct
{
uint8_t           value;
uint8_t           changed;
pressure_dir_t    dir;
uint32_t          time;
}pressure_t;



static pressure_t   pressure={
.value = 0,
.dir   = P_DIR_INIT,
.time  =0
};


static uint8_t get_pressure(uint16_t adc)
{
 float v,p;
 if(adc == ADC_TASK_ADC_ERR_VALUE){
   return PRESSURE_ERR_VALUE_SENSOR;
 }
 v= (adc*PRESSURE_SENSOR_REFERENCE_VOLTAGE)/PRESSURE_SENSOR_ADC_VALUE_MAX;   
 p=(v-PRESSURE_SENSOR_OUTPUT_VOLTAGE_MIN)*(PRESSURE_SENSOR_INPUT_PA_MAX -PRESSURE_SENSOR_INPUT_PA_MIN)/(PRESSURE_SENSOR_OUTPUT_VOLTAGE_MAX - PRESSURE_SENSOR_OUTPUT_VOLTAGE_MIN)+PRESSURE_SENSOR_INPUT_PA_MIN;
 p=(p*10)/PA_VALUE_PER_1KG_CM2;

 log_one_line("v:%d mv   p:%d kg/cm2.",(uint16_t)(v*1000),(uint32_t)p);
 if(p < PRESSURE_VALUE_IN_KG_CM2_MIN ){
 log_error("pressure over low.\r\n");
 return  PRESSURE_ERR_VALUE_OVER_LOW;
 }else if(p > PRESSURE_VALUE_IN_KG_CM2_MAX){
 log_error("pressure over high.\r\n");
 return  PRESSURE_ERR_VALUE_OVER_HIGH;
 }
 
 return (uint8_t)((uint32_t)p);
}

void pressure_task(void const *argument)
{
  uint8_t p;
  uint16_t adc;
  uint32_t cur_time;
  uint32_t delt_time;
  osEvent os_msg;
  
  osMessageQDef(pressure_msg_q,6,uint32_t);
  pressure_task_msg_q_id = osMessageCreate(osMessageQ(pressure_msg_q),pressure_task_hdl);
  log_assert(pressure_task_msg_q_id);
  
  /*等待任务同步*/
  xEventGroupSync(tasks_sync_evt_group_hdl,TASKS_SYNC_EVENT_PRESSURE_TASK_RDY,TASKS_SYNC_EVENT_ALL_TASKS_RDY,osWaitForever);
  log_debug("pressure task sync ok.\r\n");
  
  while(1){
  os_msg = osMessageGet(pressure_task_msg_q_id,PRESSURE_TASK_MSG_WAIT_TIMEOUT);
  if(os_msg.status == osEventMessage){

   ptr_msg=(task_msg_t*)os_msg.value.v;
   cur_time = osKernelSysTick(); 
   delt_time = cur_time-pressure.time;
   
   /*压力ADC完成消息处理*/
   if(ptr_msg->type == P_ADC_COMPLETED){
   adc = ptr_msg->adc;
   p = get_pressure(adc); 
   /*压力值有变化*/
   if(p !=pressure.value){
   if(p == PRESSURE_ERR_VALUE_SENSOR    ||\
      p == PRESSURE_ERR_VALUE_OVER_HIGH ||\
      p == PRESSURE_ERR_VALUE_OVER_LOW ){
    pressure.dir=P_DIR_INIT;
    pressure.value=p;
    pressure.changed=TRUE;
    log_error("pressure err.code:0x%2x.\r\n",pressure.value);
    }else if(p > pressure.value && pressure.dir == P_DIR_UP    ||\
             p < pressure.value && pressure.dir == P_DIR_DOWN  ||\
             pressure.dir == P_DIR_INIT                        ||\
             delt_time >= PRESSURE_TASK_P_HOLD_TIME ){      
   pressure.dir = p > pressure.value?P_DIR_UP:P_DIR_DOWN;
   pressure.value = p;
   pressure.time =cur_time; 
   pressure.changed=TRUE;
   }
   }
   if(pressure.changed == TRUE){
   log_debug("pressure changed. dir :%d  value:%d kg/cm2  delt_time:%d ms.\r\n" ,pressure.dir,p,delt_time);  
   pressure.changed=FALSE;
   d_msg.type = BROADCAST_PRESSURE_VALUE;
   d_msg.pressure =  pressure.value;
   osMessagePut(display_task_msg_q_id,(uint32_t)&d_msg,PRESSURE_TASK_PUT_MSG_TIMEOUT);  
   
   a_msg.type = BROADCAST_PRESSURE_VALUE;
   a_msg.pressure =  pressure.value;
   osMessagePut(alarm_task_msg_q_id,(uint32_t)&a_msg,PRESSURE_TASK_PUT_MSG_TIMEOUT);  
   }  
  }
  
  /*压力主动请求消息处理*/
  if(ptr_msg->type == REQ_PRESSURE_VALUE){
    response_msg.type = RESPONSE_PRESSURE_VALUE;
    response_msg.pressure =  pressure.value;
    osMessagePut(ptr_msg->req_q_id,(uint32_t)&response_msg,PRESSURE_TASK_PUT_MSG_TIMEOUT);  
  }
  
  
  }
 }
}