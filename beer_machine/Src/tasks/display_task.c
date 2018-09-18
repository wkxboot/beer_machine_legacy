#include "cmsis_os.h"
#include "led.h"
#include "tasks_init.h"
#include "adc_task.h"
#include "compressor_task.h"
#include "alarm_task.h"
#include "pressure_task.h"
#include "temperature_task.h"
#include "display_task.h"
#include "log.h"
#define  LOG_MODULE_LEVEL    LOG_LEVEL_DEBUG
#define  LOG_MODULE_NAME     "[dis_task]"

osThreadId   display_task_hdl;
osMessageQId display_task_msg_q_id;
osTimerId    display_timer_id;
task_msg_t   *ptr_msg;
task_msg_t   d_msg;


typedef struct
{
uint8_t            is_flash;
}circle_t;


typedef struct
{
int16_t dis_value; 
uint8_t is_flash;
}temperature_dis_t;

typedef struct
{
uint8_t dis_value; 
uint8_t is_flash;
}pressure_dis_t;

typedef struct
{
uint8_t dis_value; 
uint8_t is_flash;
}capacity_dis_t;

typedef struct
{
circle_t          circle;
temperature_dis_t temperature;
pressure_dis_t    pressure;
capacity_dis_t    capacity;
uint8_t           is_display_on;
uint8_t           is_updated;
uint16_t          time;
}display_t;


static display_t display={
.temperature.dis_value = 0,
.temperature.is_flash = FALSE,
.pressure.dis_value = 0,
.pressure.is_flash = FALSE,
.capacity.dis_value = 20,
.capacity.is_flash = FALSE,
.circle.is_flash = FALSE,
.is_display_on     = TRUE
};

static void display_timer_expired(void const *argument);

static void display_timer_init()
{
 osTimerDef(display_timer,display_timer_expired);
 display_timer_id=osTimerCreate(osTimer(display_timer),osTimerPeriodic,0);
 log_assert(display_timer_id);
}

static void display_timer_start(void)
{
 osTimerStart(display_timer_id,DISPLAY_TASK_TIMER_TIMEOUT);  
 log_debug("显示定时器开始.\r\n");
}
/*
static void display_timer_stop(void)
{
 osTimerStop(display_timer_id);  
 log_debug("显示定时器停止.\r\n");  
}
*/
static void display_timer_expired(void const *argument)
{
  d_msg.type = DISPLAY_FLASH_TIMER;
  osMessagePut(display_task_msg_q_id,(uint32_t)&d_msg,DISPLAY_TASK_PUT_MSG_TIMEOUT);
}

 

void display_task(void const *argument)
{
 osEvent os_msg
 /*等待显示芯片上电稳定*/;
 osDelay(200);
 led_display_init();
 
 led_display_temperature_unit(LED_DISPLAY_ON);
 led_display_pressure_unit(LED_DISPLAY_ON);
 led_display_capacity_unit(LED_DISPLAY_ON);
 
 led_display_temperature_icon(LED_DISPLAY_ON);
 led_display_pressure_icon(LED_DISPLAY_ON);
 led_display_capacity_icon_frame(LED_DISPLAY_ON);
 
 led_display_pressure_point(LED_DISPLAY_ON);
 led_display_capacity_icon_level(5);
 

 led_display_wifi_icon(LED_DISPLAY_ON);
 led_display_circle_icon(LED_DISPLAY_ON,LED_DISPLAY_ON);
 led_display_brand_icon(LED_DISPLAY_ON);
 
 
 
 
 /*依次显示 8-7....0.*/
 for(int8_t dis=88;dis >=0 ;dis-=11){
 led_display_temperature(dis);
 led_display_pressure(dis);
 led_display_capacity(dis);
 
 led_display_refresh();
 osDelay(250);
 }
  /*默认显示容积20L*/
  led_display_capacity(display.capacity.dis_value);
 
  osMessageQDef(display_msg_q,8,uint32_t);
  display_task_msg_q_id = osMessageCreate(osMessageQ(display_msg_q),display_task_hdl);
  log_assert(display_task_msg_q_id);
  /*等待任务同步*/
  xEventGroupSync(tasks_sync_evt_group_hdl,TASKS_SYNC_EVENT_DISPLAY_TASK_RDY,TASKS_SYNC_EVENT_ALL_TASKS_RDY,osWaitForever);
  log_debug("display task sync ok.\r\n");
  display_timer_init();
  display_timer_start();
  
  while(1){
  os_msg = osMessageGet(display_task_msg_q_id,DISPLAY_TASK_MSG_WAIT_TIMEOUT);
  if(os_msg.status == osEventMessage){
   ptr_msg = (task_msg_t *)os_msg.value.v;
   
   if(ptr_msg->type == BROADCAST_TEMPERATURE_VALUE){
    display.temperature.dis_value =ptr_msg->temperature;
    if(display.temperature.dis_value == TEMPERATURE_ERR_VALUE_OVER_HIGH||\
       display.temperature.dis_value == TEMPERATURE_ERR_VALUE_OVER_LOW ||\
       display.temperature.dis_value == TEMPERATURE_ERR_VALUE_SENSOR){
      display.temperature.is_flash = TRUE;
    }else{
      display.temperature.is_flash = FALSE;
      led_display_temperature(display.temperature.dis_value);        
      /*刷新到芯片RAM*/
     led_display_refresh();
    }
  }
   
   if(ptr_msg->type == BROADCAST_PRESSURE_VALUE){
      display.pressure.dis_value = ptr_msg->pressure;
      if(display.pressure.dis_value == PRESSURE_ERR_VALUE_OVER_HIGH ||\
         display.pressure.dis_value == PRESSURE_ERR_VALUE_OVER_LOW  ||\
         display.pressure.dis_value == PRESSURE_ERR_VALUE_SENSOR){          
     display.pressure.is_flash = TRUE;
     led_display_pressure_point(LED_DISPLAY_OFF);
     }else if(display.pressure.dis_value >= ALARM_TASK_PRESSURE_ALARM_VALUE){
     display.pressure.is_flash = TRUE;
     led_display_pressure_point(LED_DISPLAY_ON);
     }else{
     display.pressure.is_flash = FALSE;
     led_display_pressure_point(LED_DISPLAY_ON);
     led_display_pressure(display.pressure.dis_value);
     /*刷新到芯片RAM*/
     led_display_refresh();
    }
  }
    
    
  if(ptr_msg->type == BROADCAST_CAPACITY_VALUE){
    display.capacity.dis_value = ptr_msg->capacity;
    led_display_capacity(display.capacity.dis_value);
    /*刷新到芯片RAM*/
   led_display_refresh();
  }
   
   /*定时器消息*/
  if(ptr_msg->type == DISPLAY_FLASH_TIMER){
   display.time += DISPLAY_TASK_TIMER_TIMEOUT;
    
    if(display.is_display_on == TRUE){
    if(display.time >= FLASH_DISPLAY_ON_TIMEOUT){
    display.time = 0;
    display.is_display_on = FALSE;

    if(display.temperature.is_flash == TRUE){
    led_display_temperature(LED_NULL_VALUE);
    display.is_updated = TRUE;
    }
    if(display.pressure.is_flash == TRUE){
    led_display_pressure(LED_NULL_VALUE);
    display.is_updated = TRUE;
    }     
    if(display.capacity.is_flash == TRUE){
    led_display_capacity(LED_NULL_VALUE);
    display.is_updated = TRUE;
    } 
    
    if(display.circle.is_flash == TRUE){
    led_display_circle_icon(LED_DISPLAY_OFF,LED_DISPLAY_OFF);
    display.is_updated = TRUE;
    }   
    }
   }else if(display.is_display_on == FALSE){
    if(display.time >= FLASH_DISPLAY_OFF_TIMEOUT){
    display.time = 0;
    display.is_display_on = TRUE;

    if(display.temperature.is_flash == TRUE){
    led_display_temperature(display.temperature.dis_value);
    display.is_updated = TRUE;
    }
    if(display.pressure.is_flash == TRUE){
    led_display_pressure(display.pressure.dis_value);
    display.is_updated = TRUE;
    }      
    if(display.capacity.is_flash == TRUE){
    led_display_capacity(display.capacity.dis_value);
    display.is_updated = TRUE;
    }  
    if(display.circle.is_flash == TRUE){
    led_display_circle_icon(LED_DISPLAY_ON,LED_DISPLAY_ON);
    display.is_updated = TRUE;
    } 
   
  }
  }
    
 if(display.is_updated == TRUE){
    display.is_updated = FALSE;
    /*刷新到芯片RAM*/
    led_display_refresh();
    }
 }

   /*压缩机开机消息*/
   if(ptr_msg->type == COMPRESSOR_START){
    display.circle.is_flash = TRUE;
   }
   
   /*压缩机关机消息*/
   if(ptr_msg->type == COMPRESSOR_STOP){  
    display.circle.is_flash = FALSE;
    led_display_circle_icon(LED_DISPLAY_ON,LED_DISPLAY_ON);
    /*刷新到芯片RAM*/
    led_display_refresh();
   }
   
   
  }
  }
}
   
   
  
