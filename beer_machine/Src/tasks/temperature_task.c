#include "cmsis_os.h"
#include "adc_task.h"
#include "temperature_task.h"
#include "log.h"
#define  LOG_MODULE_LEVEL    LOG_LEVEL_DEBUG
#define  LOG_MODULE_NAME     "[t_task]"

osThreadId   temperature_task_hdl;
osMessageQId temperature_task_msg_q_id;

#define  TEMPERATURE_ERR_VALUE_OVER_HIGH      0xe0
#define  TEMPERATURE_ERR_VALUE_OVER_LOW       0xe1
#define  TEMPERATURE_ERR_VALUE_SENSOR         0xe3 



typedef enum
{
T_DIR_INIT=0,
T_DIR_UP,
T_DIR_DOWN=1
}temperature_dir_t;

typedef struct
{
int8_t  value;
uint8_t dir;
uint32_t time;
}temperature_t;


#define  TEMPERATURE_TASK_T_HOLD_TIME              8000
#define  TEMPERATURE_TASK_T_MAX_VALUE              35
#define  TEMPERATURE_TASK_T_MIN_VALUE              -9


static temperature_t   temperature={
.value = 0,
.dir   = T_DIR_INIT,
.time =0
};


#define  TEMPERATURE_SENSOR_FULL_VLTAGE          5.0
#define  TEMPERATURE_SENSOR_FULL_VALUE           ADC_TASK_ADC_VALUE_MAX    
#define  TEMPERATURE_SENSOR_BYPASS_RES_VALUE     5100
#define  TEMPERATURE_SENSOR_SLOPE                0.8
#define  TEMPERATURE_SENSOR_SLOPE_BASE           12.4



#define  TEMPERATURE_TASK_MSG_WAIT_TIMEOUT        osWaitForever

void temperature_task(void const *argument)
{
  uint16_t bypass_r_adc;
  uint16_t t_sensor_r;
  uint32_t cur_time;
  float temp;
  osEvent msg;
  
  osMessageQDef(temperature_msg_q,2,uint16_t);
  temperature_task_msg_q_id = osMessageCreate(osMessageQ(temperature_msg_q),temperature_task_hdl);
  log_assert(temperature_task_msg_q_id);
  
  msg = osMessageGet(temperature_task_msg_q_id,TEMPERATURE_TASK_MSG_WAIT_TIMEOUT);
  if(msg.status == osEventMessage){
   bypass_r_adc = (uint16_t)msg.value.v;
   if(bypass_r_adc == ADC_TASK_ADC_ERR_VALUE){
     temperature.value=TEMPERATURE_ERR_VALUE_SENSOR;
     temperature.time=0;
     log_error("t task sensor error.\r\n");
   }else{
     t_sensor_r= (TEMPERATURE_SENSOR_FULL_VALUE - bypass_r_adc)*TEMPERATURE_SENSOR_BYPASS_RES_VALUE/bypass_r_adc;
     temp = TEMPERATURE_SENSOR_SLOPE * t_sensor_r + TEMPERATURE_SENSOR_SLOPE_BASE;
     
     if(temp >= TEMPERATURE_TASK_T_MIN_VALUE || temp <= TEMPERATURE_TASK_T_MAX_VALUE){
       if(temperature.value > temp && temperature.dir == T_DIR_UP){
       temperature.value = (int8_t)temp;
     if(temp
     
     if(temperature.value > TEMPERATURE_TASK_T_MAX_VALUE){
      temperature.value = TEMPERATURE_ERR_VALUE_OVER_HIGH;
      temperature.time=0;
     }
     if(temperature.value < TEMPERATURE_TASK_T_MIN_VALUE){
      temperature.value = TEMPERATURE_ERR_VALUE_OVER_LOW;
      temperature.time=0;
     }
   
   
   
  }
}
}