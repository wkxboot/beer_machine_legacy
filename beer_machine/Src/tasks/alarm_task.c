#include "cmsis_os.h"
#include "tasks_init.h"
#include "temperature_task.h"
#include "pressure_task.h"
#include "beer_machine.h"
#include "alarm_task.h"
#include "log.h"
#define  LOG_MODULE_LEVEL    LOG_LEVEL_DEBUG
#define  LOG_MODULE_NAME     "[alarm_task]"

osThreadId   alarm_task_hdl;
osMessageQId alarm_task_msg_q_id;
osTimerId    alarm_timer_id;

static alarm_msg_t  a_msg;
static alarm_msg_t *ptr_msg;

typedef enum
{
ALARM_STATUS_ON_FIRST=0,        /*警报第1次开启*/
ALARM_STATUS_ON_SECOND,         /*警报第2次开启*/
ALARM_STATUS_OFF,               /*警报关闭*/
ALARM_STATUS_CLEAR_FIRST_START, /*警报第1次解除*/
ALARM_STATUS_CLEAR_FIRST_END,   /*警报第1次解除经过指定时间后的状态*/
ALARM_STATUS_CLEAR_SECOND       /*警报第2次解除*/
}alarm_status_t;


typedef enum
{
BUZZER_STATUS_PWR_INIT=0,
BUZZER_STATUS_PWR_ON,
BUZZER_STATUS_PWR_OFF,
}buzzer_status_t;

typedef struct
{
buzzer_status_t pwr;
uint32_t        on_time;
uint32_t        off_time;
}buzzer_t;

typedef enum
{
SW_STATUS_INIT=0,
SW_STATUS_PRESS,
SW_STATUS_RELEASE
}switch_status_t;

typedef struct
{
switch_status_t status;
uint32_t        press_time;
}switch_t;

typedef struct 
{
buzzer_t       buzzer;
switch_t       sw;
alarm_status_t status;
uint32_t       time;
}pressure_alarm_t;




static pressure_alarm_t  alarm;



/*蜂鸣器通电时间 单位:ms*/
#define  ALARM_TASK_ALARM_PWR_ON_TIMEOUT          (10*ALARM_TASK_ALARM_DUTY_ON_RATE)/ALARM_TASK_ALARM_RATE_HZ
/*蜂鸣器断电时间 单位:ms*/ 
#define  ALARM_TASK_ALARM_PWR_OFF_TIMEOUT         (10*(100-ALARM_TASK_ALARM_DUTY_ON_RATE))/ALARM_TASK_ALARM_RATE_HZ





static void alarm_timer_expired(void const * argument);
static void alarm_buzzer_pwr_turn_on();
static void alarm_buzzer_pwr_turn_off();

static void alarm_buzzer_pwr_turn_on()
{
  bsp_buzzer_ctrl_on();
}

static void alarm_buzzer_pwr_turn_off()
{
  bsp_buzzer_ctrl_on();
}

static void alarm_timer_init()
{
 osTimerDef(alarm_timer,alarm_timer_expired);
 alarm_timer_id=osTimerCreate(osTimer(alarm_timer),osTimerOnce,0);
 log_assert(alarm_timer_id);
}

static void alarm_start_first_timer(void)
{
 osTimerStart(alarm_timer_id,ALARM_TASK_ALARM_ON_FIRST_TIMEOUT);  
 log_debug("启动第1次报警定时器.\r\n");
}

static void alarm_start_wait_timer(void)
{
 osTimerStart(alarm_timer_id,ALARM_TASK_ALARM_WAIT_TIMEOUT);
 log_debug("启动第2次报警等待定时器.\r\n");
}


static void alarm_timer_stop()
{
 osTimerStop(alarm_timer_id);
 log_debug("报警定时器停止.\r\n");
}


static void alarm_timer_expired(void const * argument)
{
 if(alarm.status == ALARM_STATUS_ON_FIRST){
  log_debug("超时自动解除第1次报警.\r\n");
  alarm.status = ALARM_STATUS_CLEAR_FIRST_START; 
  alarm_start_wait_timer();
 }else if(alarm.status == ALARM_STATUS_CLEAR_FIRST_START){
  alarm.status = ALARM_STATUS_CLEAR_FIRST_END; 
  log_debug("请求压力值.\r\n");
 }
}



void alarm_task(void const *argument)
{
  uint8_t pressure;
  uint32_t cur_time;
  osEvent os_msg;
  
  osMessageQDef(alarm_msg_q,4,uint32_t);
  alarm_task_msg_q_id = osMessageCreate(osMessageQ(alarm_msg_q),alarm_task_hdl);
  log_assert(alarm_task_msg_q_id);
  alarm_timer_init();
  
  /*等待任务同步*/
  xEventGroupSync(tasks_sync_evt_group_hdl,TASKS_SYNC_EVENT_ALARM_TASK_RDY,TASKS_SYNC_EVENT_ALL_TASKS_RDY,osWaitForever);
  log_debug("alarm task sync ok.\r\n");
  
  while(1){
  os_msg = osMessageGet(pressure_task_msg_q_id,ALARM_TASK_MSG_WAIT_TIMEOUT);
  cur_time = osKernelSysTick();
  if(os_msg.status == osEventMessage){
  ptr_msg = (alarm_msg_t *)os_msg.value.v;
  
  /*压力值处理*/
  if(ptr_msg->type == ALARM_PRESSURE_VALUE){
   pressure=ptr_msg->value;
   if(pressure >= ALARM_TASK_PRESSURE_ALARM_VALUE){
    if(alarm.status == ALARM_STATUS_OFF){
    alarm.status = ALARM_STATUS_ON_FIRST;
    alarm_start_first_timer();
    log_warning("压力:%d kg/cm2过高，第1次报警.\r\n",pressure);
    }else if(alarm.status == ALARM_STATUS_CLEAR_FIRST_END){
     alarm.status = ALARM_STATUS_ON_SECOND;
     log_warning("压力:%d kg/cm2过高，第2次报警.\r\n",pressure);
    }
   }else if(alarm.status != ALARM_STATUS_OFF){
    alarm.status = ALARM_STATUS_OFF; 
    alarm_timer_stop();
    log_debug("压力%d kg/cm2恢复正常，解除报警.\r\n",pressure);
   }
  }
  
  /*短按键消息处理*/
  if(ptr_msg->type == ALARM_SW_SHORT_PRESS){
    if(alarm.status == ALARM_STATUS_ON_FIRST){
     log_debug("短按.解除第1次报警.\r\n");
     alarm.status = ALARM_STATUS_CLEAR_FIRST_START; 
     alarm_start_wait_timer();
    }else{
     log_debug("短按.无效.\r\n");
    }
  }
  
  
  /*长按键消息处理*/
  if(ptr_msg->type == ALARM_SW_LONG_PRESS){
   if(alarm.status == ALARM_STATUS_ON_SECOND){
   alarm.status = ALARM_STATUS_CLEAR_SECOND; 
   log_debug("长按.解除第2次报警.\r\n");
   }else{
   log_debug("长按.无效.\r\n");
   }
  } 
  }
  
  
 /*蜂鸣器状态处理*/
  if(alarm.status == ALARM_STATUS_ON_FIRST || alarm.status == ALARM_STATUS_ON_SECOND){
    if(alarm.buzzer.pwr == BUZZER_STATUS_PWR_ON){
      alarm.buzzer.on_time+=cur_time-alarm.time;
      if(alarm.buzzer.on_time > ALARM_TASK_ALARM_PWR_ON_TIMEOUT){
      alarm_buzzer_pwr_turn_off();
      alarm.buzzer.on_time=0;
      alarm.buzzer.pwr = BUZZER_STATUS_PWR_OFF;
      }
    }else if(alarm.buzzer.pwr == BUZZER_STATUS_PWR_OFF){
      alarm.buzzer.off_time+=cur_time-alarm.time;;
      if(alarm.buzzer.off_time > ALARM_TASK_ALARM_PWR_OFF_TIMEOUT){
      alarm_buzzer_pwr_turn_on();
      alarm.buzzer.off_time=0;
      alarm.buzzer.pwr = BUZZER_STATUS_PWR_ON; 
      }
    }    
  }else if(alarm.buzzer.pwr != BUZZER_STATUS_PWR_OFF ){
      alarm_buzzer_pwr_turn_off();
      alarm.buzzer.pwr = BUZZER_STATUS_PWR_OFF;
  } 
  
  
 /*按键状态监视处理*/ 
  if( bsp_is_alarm_sw_press()){
    if(alarm.sw.status !=SW_STATUS_PRESS){
    alarm.sw.status=SW_STATUS_PRESS;
    alarm.sw.press_time =0;
    }else{
    alarm.sw.press_time+=cur_time - alarm.time;
    if(alarm.sw.press_time >= ALARM_TASK_SW_LONG_PRESS_TIMEOUT){
    log_debug("确认一次长按键.\r\n");
    a_msg.type = ALARM_SW_LONG_PRESS;
    osMessagePut(alarm_task_msg_q_id,(uint32_t)&a_msg,0); 
    }
    }
  }else if(alarm.sw.status !=SW_STATUS_RELEASE){
   alarm.sw.status =  SW_STATUS_RELEASE;
   /*当松开按键时 如果pree_time大于短按时间且小于长按时间，认为是短按.大于长按的时间忽略*/
   if(alarm.sw.press_time >= ALARM_TASK_SW_SHORT_PRESS_TIMEOUT && alarm.sw.press_time < ALARM_TASK_SW_LONG_PRESS_TIMEOUT){
    log_debug("确认一次短按键.\r\n");
    a_msg.type = ALARM_SW_SHORT_PRESS;
    osMessagePut(alarm_task_msg_q_id,(uint32_t)&a_msg,0);  
    }
  }
  
  alarm.time=cur_time;
  }
}