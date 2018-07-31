#include "cmsis_os.h"
#include "adc_task.h"
#include "compressor_task.h"
#include "temperature_task.h"
#include "log.h"
#define  LOG_MODULE_LEVEL    LOG_LEVEL_DEBUG
#define  LOG_MODULE_NAME     "[t_task]"

osThreadId   temperature_task_hdl;
osMessageQId temperature_task_msg_q_id;

static compressor_msg_t c_msg;



static int16_t const t_r_map[][2]={
  {-12,12160},{-11,11520},{-10,10920},{-9,10350},{-8,9820},{-7,9316},{-6,8841},{-5,8392},{-4,7968},
  {-3 ,7568} ,{-2 ,7190} ,{-1,6833}  ,{0,6495}  ,{1,6175 },{2,5873 },{3,5587 },{4,5315} ,{5,5060},
  {6,4818}   ,{7,4589}   ,{8,4372}   ,{9,4167}  ,{10,3972},{11,3788},{12,3613},{13,3447},{14,3290},
  {15,3141}  ,{16,2999}  ,{17,2865}  ,{18,2737} ,{19,2616},{20,2501},{21,2391},{22,2287},{23,2188},
  {24,2094}  ,{25,2005}  ,{26,1919}  ,{27,1838} ,{28,1761},{29,1687},{30,1617},{31,1550},{32,1486},
  {33,1426}  ,{34,1368}  ,{35,1312}  ,{36,1259} ,{37,1209},{38,1161},{39,1115},{40,1071},{41,1029},
  {42,9885}  ,{43,9506}  ,{44,9140}  ,{45,8789} ,{46,8454},{47,8133},{48,7826},{49,7532},{50,7251}
};


#define  TR_MAP_IDX_MIN                    0  /*t_r_map[0 ][1] r=12160 -12摄氏度*/ 
#define  TR_MAP_IDX_MAX                    62 /*t_r_map[62][1] r=7251   50摄氏度*/ 

#define  TR_MAP_IDX_OVER_HIGH_ERR          0xff
#define  TR_MAP_IDX_OVER_LOW_ERR           0xfe


typedef enum
{
T_DIR_INIT=0,
T_DIR_UP,
T_DIR_DOWN=1
}temperature_dir_t;

typedef struct
{
int16_t           value;
temperature_dir_t dir;
uint32_t          time;
uint8_t           changed;
}temperature_t;



static temperature_t   temperature={
.value = 0,
.dir   = T_DIR_INIT,
.time =0,
.changed=0
};
               


static uint8_t seek_idex(uint32_t r)
{
 uint8_t mid=0;
 int low = TR_MAP_IDX_MIN;  
 int high =TR_MAP_IDX_MAX;  
 
 if(r < t_r_map[TR_MAP_IDX_MAX][1]){
 log_error("NTC 阻值超过最高温度范围！r=%d\r\n",r); 
 return TR_MAP_IDX_OVER_HIGH_ERR;
 }else if(r > t_r_map[TR_MAP_IDX_MIN][1]){
 log_error("NTC 阻值超过最低温度范围！r=%d\r\n",r); 
 return TR_MAP_IDX_OVER_LOW_ERR;
 }
 
  while (low <= high) {  
  mid = (low + high) / 2;  
  if(r > t_r_map[mid][1]){
    if(r <= t_r_map[mid-1][1]){
    return mid - 1;
    } else{
    high = mid - 1;  
    }
  } else {
  if(r > t_r_map[mid+1][1]){
   return mid;
  } else{
  low = mid;   
  }
  }  
 }
 return 0; 
}




static uint32_t get_r(uint16_t adc)
{
  float t_sensor_r;
  t_sensor_r = (TEMPERATURE_SENSOR_SUPPLY_VOLTAGE*TEMPERATURE_SENSOR_ADC_VALUE_MAX*TEMPERATURE_SENSOR_BYPASS_RES_VALUE)/(adc*TEMPERATURE_SENSOR_REFERENCE_VOLTAGE)-TEMPERATURE_SENSOR_BYPASS_RES_VALUE;
  return (uint32_t)t_sensor_r;
}

int16_t get_t(uint16_t adc)
{
  uint32_t r; 
  uint8_t idx;
  if(adc == ADC_TASK_ADC_ERR_VALUE){
  return TEMPERATURE_ERR_VALUE_SENSOR;
  }
  r=get_r(adc);
  idx = seek_idex(r);
  if(idx == TR_MAP_IDX_OVER_HIGH_ERR){
  return TEMPERATURE_ERR_VALUE_OVER_HIGH;
  }else if(idx == TR_MAP_IDX_OVER_HIGH_ERR){
  return TEMPERATURE_ERR_VALUE_OVER_LOW;
  }
  return t_r_map[idx][0];
}

void temperature_task(void const *argument)
{
  temperature_msg_t *t_msg;
  uint16_t bypass_r_adc;
  uint32_t cur_time;
  int16_t  temp;
  osEvent  msg;
  
  osMessageQDef(temperature_msg_q,2,uint16_t);
  temperature_task_msg_q_id = osMessageCreate(osMessageQ(temperature_msg_q),temperature_task_hdl);
  log_assert(temperature_task_msg_q_id);
  
  while(1){
  msg = osMessageGet(temperature_task_msg_q_id,TEMPERATURE_TASK_MSG_WAIT_TIMEOUT);
  if(msg.status == osEventMessage){
  t_msg = (temperature_msg_t *)msg.value.v;
  
  /*温度ADC转换完成消息*/
  if(t_msg->type == T_ADC_COMPLETED){
   cur_time = osKernelSysTick(); 
   bypass_r_adc = t_msg->value;
   temp =get_t(bypass_r_adc);
   
   if(temp == TEMPERATURE_ERR_VALUE_SENSOR    ||\
      temp == TEMPERATURE_ERR_VALUE_OVER_HIGH ||\
      temp == TEMPERATURE_ERR_VALUE_OVER_LOW ){
   temperature.dir=T_DIR_INIT;
   temperature.value=temp;
   temperature.time=0;
   temperature.changed=1;
   log_error("temperature err.code:%d.\r\n",temperature.value);
   }else if(temp > temperature.value && temperature.dir == T_DIR_UP    ||\
            temp < temperature.value && temperature.dir == T_DIR_DOWN  ||\
            temperature.dir == T_DIR_INIT                              ||\
            cur_time - temperature.time >= TEMPERATURE_TASK_T_HOLD_TIME ){         
   temperature.dir = temp > temperature.value?T_DIR_UP:T_DIR_DOWN;
   temperature.value = (int16_t)temp;
   temperature.time =cur_time; 
   temperature.changed=1;
   log_debug("t ok:%d ℃.\r\n",temperature.value);
   }
   
   if(temperature.changed){
   log_debug("t changed:%d ℃.\r\n",temperature.value);
   temperature.changed=0;
   c_msg.type = TEMPERATURE_VALUE;
   c_msg.value= temperature.value;
   osMessagePut(compressor_task_msg_q_id,(uint32_t)&c_msg,0);
   }
  }
  /*请求温度消息*/
  if(t_msg->type == REQ_TEMPERATURE){
   c_msg.type = TEMPERATURE_VALUE;
   c_msg.value= temperature.value;
   osMessagePut(compressor_task_msg_q_id,(uint32_t)&c_msg,0); 
  }
 }
 }
}