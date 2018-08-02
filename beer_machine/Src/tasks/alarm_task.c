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
osTimerId    alarm_switch_timer_id;
osTimerId    alarm_buzzer_timer_id;

static task_msg_t  a_msg;
static task_msg_t  p_msg;
static task_msg_t  *ptr_msg;

typedef enum
{
ALARM_STATUS_ON_FIRST=0,        /*警报第1次开启*/
ALARM_STATUS_ON_SECOND,         /*警报第2次开启*/
ALARM_STATUS_OFF,               /*警报关闭*/
ALARM_STATUS_CLEAR_WAIT,        /*警报第1次解除后的等待状态*/
ALARM_STATUS_CLEAR_CONFIRM,     /*警报第1次解除后再次等待确认压力的状态*/
ALARM_STATUS_CLEAR_SECOND       /*警报第2次解除*/
}alarm_status_t;


typedef enum
{
BUZZER_PWR_INIT=0,
BUZZER_PWR_ON,
BUZZER_PWR_OFF,
}buzzer_power_t;

typedef struct
{
buzzer_power_t  pwr;
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




static void alarm_switch_timer_expired(void const * argument);
static void alarm_buzzer_timer_expired(void const * argument);
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

static void alarm_switch_timer_init()
{
 osTimerDef(alarm_switch_timer,alarm_switch_timer_expired);
 alarm_switch_timer_id=osTimerCreate(osTimer(alarm_switch_timer),osTimerPeriodic,&alarm);
 log_assert(alarm_switch_timer_id);
}

static void alarm_switch_timer_start(void)
{
 osTimerStart(alarm_switch_timer_id,ALARM_TASK_SWITCH_MONITOR_TIMEOUT);  
 log_debug("启动按键监视定时器.\r\n");
}

static void alarm_switch_timer_expired(void const * argument)
{
  pressure_alarm_t *ptr_alarm; 
  ptr_alarm=&alarm;  
 /*按键状态监视处理*/ 
  if( bsp_is_alarm_sw_press()){
    if(ptr_alarm->sw.status !=SW_STATUS_PRESS){
    ptr_alarm->sw.status=SW_STATUS_PRESS;
    ptr_alarm->sw.press_time =0;
    }else{
    ptr_alarm->sw.press_time+=ALARM_TASK_SWITCH_MONITOR_TIMEOUT;
    if(ptr_alarm->sw.press_time >= ALARM_TASK_SW_LONG_PRESS_TIMEOUT &&   \
       ptr_alarm->sw.press_time < ALARM_TASK_SW_LONG_PRESS_TIMEOUT+ALARM_TASK_SWITCH_MONITOR_TIMEOUT){
    log_debug("确认一次长按键.\r\n");
    a_msg.type = ALARM_SW_LONG_PRESS;
    osMessagePut(alarm_task_msg_q_id,(uint32_t)&a_msg,0); 
    }
    }
  }else if(ptr_alarm->sw.status !=SW_STATUS_RELEASE){
   ptr_alarm->sw.status =  SW_STATUS_RELEASE;
   /*当松开按键时 如果pree_time大于短按时间且小于长按时间，认为是短按.大于长按的时间忽略*/
   if(ptr_alarm->sw.press_time >= ALARM_TASK_SW_SHORT_PRESS_TIMEOUT && ptr_alarm->sw.press_time < ALARM_TASK_SW_LONG_PRESS_TIMEOUT){
    log_debug("确认一次短按键.\r\n");
    a_msg.type = ALARM_SW_SHORT_PRESS;
    osMessagePut(alarm_task_msg_q_id,(uint32_t)&a_msg,0);  
    }
  }
}

static void alarm_buzzer_timer_init()
{
 osTimerDef(alarm_buzzer_timer,alarm_buzzer_timer_expired);
 alarm_buzzer_timer_id=osTimerCreate(osTimer(alarm_buzzer_timer),osTimerPeriodic,&alarm);
 log_assert(alarm_buzzer_timer_id);
}

static void alarm_buzzer_timer_start(void)
{
 osTimerStart(alarm_buzzer_timer_id,ALARM_TASK_BUZZER_MONITOR_TIMEOUT);  
 log_debug("启动蜂鸣器监视定时器.\r\n");
}

static void alarm_buzzer_timer_expired(void const * argument)
{
  pressure_alarm_t *ptr_alarm;
  ptr_alarm=&alarm;
  
   /*蜂鸣器状态处理*/
  if(ptr_alarm->status == ALARM_STATUS_ON_FIRST || ptr_alarm->status == ALARM_STATUS_ON_SECOND){
    if(ptr_alarm->buzzer.pwr == BUZZER_PWR_ON){
      ptr_alarm->buzzer.on_time+=ALARM_TASK_BUZZER_MONITOR_TIMEOUT;
      if(ptr_alarm->buzzer.on_time > ALARM_TASK_ALARM_PWR_ON_TIMEOUT){
      alarm_buzzer_pwr_turn_off();
      ptr_alarm->buzzer.on_time=0;
      ptr_alarm->buzzer.pwr = BUZZER_PWR_OFF;
      }
    }else if(ptr_alarm->buzzer.pwr == BUZZER_PWR_OFF){
      ptr_alarm->buzzer.off_time+=ALARM_TASK_BUZZER_MONITOR_TIMEOUT;
      if(ptr_alarm->buzzer.off_time > ALARM_TASK_ALARM_PWR_OFF_TIMEOUT){
      alarm_buzzer_pwr_turn_on();
      ptr_alarm->buzzer.off_time=0;
      ptr_alarm->buzzer.pwr = BUZZER_PWR_ON; 
      }
    }    
  }else if(ptr_alarm->buzzer.pwr != BUZZER_PWR_OFF ){
      alarm_buzzer_pwr_turn_off();
      ptr_alarm->buzzer.pwr = BUZZER_PWR_OFF;
  } 
  
}

static void alarm_timer_init()
{
 osTimerDef(alarm_timer,alarm_timer_expired);
 alarm_timer_id=osTimerCreate(osTimer(alarm_timer),osTimerOnce,&alarm);
 log_assert(alarm_timer_id);
}

static void alarm_timer_start(void)
{
 osTimerStart(alarm_timer_id,ALARM_TASK_ALARM_ON_FIRST_TIMEOUT);  
 log_debug("启动第1次报警定时器.\r\n");
}

static void alarm_timer_wait_start(void)
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
  osStatus status;
  pressure_alarm_t *ptr_alarm;
  
  ptr_alarm=&alarm;
 if(ptr_alarm->status == ALARM_STATUS_ON_FIRST){
  log_debug("超时自动解除第1次报警.\r\n");
  ptr_alarm->status = ALARM_STATUS_CLEAR_WAIT; 
  alarm_timer_wait_start();
 }else if(ptr_alarm->status == ALARM_STATUS_CLEAR_WAIT){
  ptr_alarm->status = ALARM_STATUS_CLEAR_CONFIRM; 
  log_debug("请求压力值.\r\n");
  p_msg.type = REQ_PRESSURE_VALUE;
  p_msg.req_q_id = alarm_task_msg_q_id;
  
  status = osMessagePut(pressure_task_msg_q_id,(uint32_t)&p_msg,ALARM_TASK_MSG_SEND_TIMEOUT);
  if(status !=osOK){
  log_error("alarm req p msg err.\r\n");
  }
  
 }
}


void alarm_task(void const *argument)
{
  uint8_t pressure;
  pressure_alarm_t *ptr_alarm;
  osEvent os_msg;
 
  ptr_alarm=&alarm;
  
  osMessageQDef(alarm_msg_q,4,uint32_t);
  alarm_task_msg_q_id = osMessageCreate(osMessageQ(alarm_msg_q),alarm_task_hdl);
  log_assert(alarm_task_msg_q_id);
 
  /*等待任务同步*/
  xEventGroupSync(tasks_sync_evt_group_hdl,TASKS_SYNC_EVENT_ALARM_TASK_RDY,TASKS_SYNC_EVENT_ALL_TASKS_RDY,osWaitForever);
  log_debug("alarm task sync ok.\r\n");
  
  alarm_timer_init();
  alarm_switch_timer_init();
  alarm_switch_timer_start();
  alarm_buzzer_timer_init();
  alarm_buzzer_timer_start();
  
  while(1){ 
  os_msg = osMessageGet(alarm_task_msg_q_id,ALARM_TASK_MSG_WAIT_TIMEOUT); 
  if(os_msg.status == osEventMessage){
  ptr_msg = (task_msg_t *)os_msg.value.v;
  
  /*压力值处理*/
  if(ptr_msg->type == RESPONSE_PRESSURE_VALUE || ptr_msg->type == BROADCAST_PRESSURE_VALUE){
   pressure=ptr_msg->pressure;
   if(pressure >= ALARM_TASK_PRESSURE_ALARM_VALUE){
    if(ptr_alarm->status == ALARM_STATUS_OFF){
    ptr_alarm->status = ALARM_STATUS_ON_FIRST;
    alarm_timer_start();
    log_warning("压力:%d kg/cm2过高，第1次报警.\r\n",pressure);
    }else if(ptr_alarm->status == ALARM_STATUS_CLEAR_CONFIRM){
     ptr_alarm->status = ALARM_STATUS_ON_SECOND;
     log_warning("压力:%d kg/cm2过高，第2次报警.\r\n",pressure);
    }
   }else if(ptr_alarm->status != ALARM_STATUS_OFF){
    ptr_alarm->status = ALARM_STATUS_OFF; 
    alarm_timer_stop();
    log_debug("压力%d kg/cm2恢复正常，解除报警.\r\n",pressure);
   }
  }
  
  /*短按键消息处理*/
  if(ptr_msg->type == ALARM_SW_SHORT_PRESS){
    if(ptr_alarm->status == ALARM_STATUS_ON_FIRST){
     log_debug("短按.解除第1次报警.\r\n");
     ptr_alarm->status = ALARM_STATUS_CLEAR_WAIT; 
     alarm_timer_wait_start();
    }else{
     log_debug("短按.无效.\r\n");
    }
  }
  
  
  /*长按键消息处理*/
  if(ptr_msg->type == ALARM_SW_LONG_PRESS){
   if(ptr_alarm->status == ALARM_STATUS_ON_SECOND){
   ptr_alarm->status = ALARM_STATUS_CLEAR_SECOND; 
   log_debug("长按.解除第2次报警.\r\n");
   }else{
   log_debug("长按.无效.\r\n");
   }
  } 
  
  }
  }
}