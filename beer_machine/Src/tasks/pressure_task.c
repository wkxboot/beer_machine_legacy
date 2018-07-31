#include "cmsis_os.h"
#include "adc_task.h"
#include "pressure_task.h"
#include "log.h"
#define  LOG_MODULE_LEVEL    LOG_LEVEL_DEBUG
#define  LOG_MODULE_NAME     "[p_task]"

osThreadId   pressure_task_hdl;
osMessageQId pressure_task_msg_q_id;

typedef enum
{
P_DIR_INIT=0,
P_DIR_UP,
P_DIR_DOWN=1
}pressure_dir_t;

typedef struct
{
uint8_t           value;
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

 log_debug("p:%d kg/cm2.\r\n",(uint8_t)p);
 if(p < 0 ){
 log_error("pressure over low.\r\n");
 return  PRESSURE_ERR_VALUE_OVER_LOW;
 }else if(p > PRESSURE_VALUE_IN_KG_CM2_MAX){
 log_error("pressure over high.\r\n");
 return  PRESSURE_ERR_VALUE_OVER_HIGH;
 }
 
 return (uint8_t)p;
}

void pressure_task(void const *argument)
{
  uint8_t temp;
  uint16_t p_adc;
  uint32_t cur_time;
  osEvent msg;
  
  osMessageQDef(pressure_msg_q,2,uint16_t);
  pressure_task_msg_q_id = osMessageCreate(osMessageQ(pressure_msg_q),pressure_task_hdl);
  log_assert(pressure_task_msg_q_id);
  
  while(1){
  msg = osMessageGet(pressure_task_msg_q_id,PRESSURE_TASK_MSG_WAIT_TIMEOUT);
  if(msg.status == osEventMessage){
   cur_time = osKernelSysTick(); 
   p_adc = (uint16_t)msg.value.v;
   temp = get_pressure(p_adc);
   
   if(temp == PRESSURE_ERR_VALUE_SENSOR    ||\
      temp == PRESSURE_ERR_VALUE_OVER_HIGH ||\
      temp == PRESSURE_ERR_VALUE_OVER_LOW ){
    pressure.dir=P_DIR_INIT;
    pressure.value=temp;
    pressure.time=0;
    log_error("pressure err.code:%d.\r\n",pressure.value);
    }else if(temp > pressure.value && pressure.dir == P_DIR_UP    ||\
             temp < pressure.value && pressure.dir == P_DIR_DOWN  ||\
             pressure.dir == P_DIR_INIT                           ||\
             cur_time - pressure.time >= PRESSURE_TASK_P_HOLD_TIME ){         
   pressure.dir = temp > pressure.value?P_DIR_UP:P_DIR_DOWN;
   pressure.value = (int16_t)temp;
   pressure.time =cur_time; 
   log_debug("p changed:%d ℃.\r\n",pressure.value);
   }
  }
 }
}