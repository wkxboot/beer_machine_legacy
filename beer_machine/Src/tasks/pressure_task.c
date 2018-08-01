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

static pressure_msg_t *ptr_msg;
static display_msg_t   d_msg;
static alarm_msg_t     a_msg;


typedef enum
{
P_DIR_INIT=0,
P_DIR_UP,
P_DIR_DOWN=1
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
 log_debug("v:%d mv.\r\n",(uint16_t)(v*1000));
     
 p=(v-PRESSURE_SENSOR_OUTPUT_VOLTAGE_MIN)*(PRESSURE_SENSOR_INPUT_PA_MAX -PRESSURE_SENSOR_INPUT_PA_MIN)/(PRESSURE_SENSOR_OUTPUT_VOLTAGE_MAX - PRESSURE_SENSOR_OUTPUT_VOLTAGE_MIN)+PRESSURE_SENSOR_INPUT_PA_MIN;
 p=(p*10)/PA_VALUE_PER_1KG_CM2;

 log_debug("p:%d kg/cm2.\r\n",(uint32_t)p);
 if(p < 0 ){
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
  osEvent os_msg;
  
  osMessageQDef(pressure_msg_q,4,uint32_t);
  pressure_task_msg_q_id = osMessageCreate(osMessageQ(pressure_msg_q),pressure_task_hdl);
  log_assert(pressure_task_msg_q_id);
  
  /*等待任务同步*/
  xEventGroupSync(tasks_sync_evt_group_hdl,TASKS_SYNC_EVENT_PRESSURE_TASK_RDY,TASKS_SYNC_EVENT_ALL_TASKS_RDY,osWaitForever);
  log_debug("pressure task sync ok.\r\n");
  
  while(1){
  os_msg = osMessageGet(pressure_task_msg_q_id,PRESSURE_TASK_MSG_WAIT_TIMEOUT);
  if(os_msg.status == osEventMessage){

   cur_time = osKernelSysTick(); 
   ptr_msg=(pressure_msg_t*)os_msg.value.v;
   if(ptr_msg->type == P_ADC_COMPLETED){
   adc = ptr_msg->value;
   p = get_pressure(adc);  
   if(p == PRESSURE_ERR_VALUE_SENSOR    ||\
      p == PRESSURE_ERR_VALUE_OVER_HIGH ||\
      p == PRESSURE_ERR_VALUE_OVER_LOW ){
    pressure.dir=P_DIR_INIT;
    pressure.value=p;
    pressure.time=0;
    pressure.changed=1;
    log_error("pressure err.code:%d.\r\n",pressure.value);
    }else if(p > pressure.value && pressure.dir == P_DIR_UP    ||\
             p < pressure.value && pressure.dir == P_DIR_DOWN  ||\
             pressure.dir == P_DIR_INIT                           ||\
             cur_time - pressure.time >= PRESSURE_TASK_P_HOLD_TIME ){         
   pressure.dir = p > pressure.value?P_DIR_UP:P_DIR_DOWN;
   pressure.value = p;
   pressure.time =cur_time; 
   pressure.changed=1;
   log_debug("p changed:%d kg/cm2.\r\n",pressure.value);
   }
   
   if(pressure.changed){
   pressure.changed=0;
   d_msg.type = DIS_PRESSURE_VALUE;
   d_msg.value =  pressure.value;
   osMessagePut(display_task_msg_q_id,(uint32_t)&d_msg,0);  
   
   a_msg.type = ALARM_PRESSURE_VALUE;
   a_msg.value =  pressure.value;
   osMessagePut(alarm_task_msg_q_id,(uint32_t)&a_msg,0);  
   }
   
  }
  
  }
 }
}