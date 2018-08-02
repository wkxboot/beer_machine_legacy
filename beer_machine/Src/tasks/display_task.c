#include "cmsis_os.h"
#include "led.h"
#include "tasks_init.h"
#include "adc_task.h"
#include "compressor_task.h"
#include "temperature_task.h"
#include "display_task.h"
#include "log.h"
#define  LOG_MODULE_LEVEL    LOG_LEVEL_DEBUG
#define  LOG_MODULE_NAME     "[dis_task]"

osThreadId   display_task_hdl;
osMessageQId display_task_msg_q_id;

task_msg_t *ptr_msg;




void display_task(void const *argument)
{
  uint8_t capacity;
  uint8_t pressure;
  int16_t temperature;
  
  osEvent os_msg;
  
  led_display_init();
  
  osMessageQDef(display_msg_q,4,uint32_t);
  display_task_msg_q_id = osMessageCreate(osMessageQ(display_msg_q),display_task_hdl);
  log_assert(display_task_msg_q_id);
  /*等待任务同步*/
  xEventGroupSync(tasks_sync_evt_group_hdl,TASKS_SYNC_EVENT_DISPLAY_TASK_RDY,TASKS_SYNC_EVENT_ALL_TASKS_RDY,osWaitForever);
  log_debug("display task sync ok.\r\n");
  
  while(1){
  os_msg = osMessageGet(display_task_msg_q_id,DISPLAY_TASK_MSG_WAIT_TIMEOUT);
  if(os_msg.status == osEventMessage){
   ptr_msg = (task_msg_t *)os_msg.value.v;
   if(ptr_msg->type == BROADCAST_TEMPERATURE_VALUE){
    temperature =ptr_msg->temperature;
    led_display_temperature(temperature);
   }
   
   if(ptr_msg->type == BROADCAST_PRESSURE_VALUE){
     pressure = ptr_msg->pressure;
     led_display_pressure(pressure);
   }
   
   if(ptr_msg->type == BROADCAST_CAPACITY_VALUE){
     capacity = ptr_msg->capacity;
     led_display_pressure(capacity);
   }
   
   
   
   /*刷新到芯片RAM*/
   led_display_refresh();
  }
  }
}
   
   
  
