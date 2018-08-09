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

osTimerId    pressure_alarm_on_timer_id;
osTimerId    pressure_alarm_confirm_timer_id;
osTimerId    temperature_alarm_on_timer_id;
osTimerId    temperature_alarm_confirm_timer_id;

osTimerId    alarm_switch_timer_id;
osTimerId    alarm_buzzer_timer_id;


static task_msg_t  asw_msg;/*报警按键消息体*/

static task_msg_t  p_msg;
static task_msg_t  t_msg;
static task_msg_t  *ptr_msg;


typedef enum
{
ALARM_ON_WAIT_CLEAR=0,    
ALARM_CLEAR_WAIT_TIMEOUT,  
ALARM_TIMEOUT_WAIT_CONFIRM,
ALARM_CONFIRM_WAIT_CLEAR,  
ALARM_CLEAR_WAIT_OFF,      
ALARM_OFF                  
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
osMutexId      mutex;
buzzer_t       buzzer;
switch_t       sw;
alarm_status_t p_status;
alarm_status_t t_status;
uint32_t       time;
}alarm_t;


static  alarm_t  alarm={
.p_status =ALARM_OFF,
.t_status =ALARM_OFF,
};

static alarm_t  *ptr_alarm = &alarm;


/*蜂鸣器通电时间 单位:ms*/
#define  ALARM_TASK_ALARM_PWR_ON_TIMEOUT          (10*ALARM_TASK_ALARM_DUTY_ON_RATE)/ALARM_TASK_ALARM_RATE_HZ
/*蜂鸣器断电时间 单位:ms*/ 
#define  ALARM_TASK_ALARM_PWR_OFF_TIMEOUT         (10*(100-ALARM_TASK_ALARM_DUTY_ON_RATE))/ALARM_TASK_ALARM_RATE_HZ




static void alarm_switch_timer_expired(void const * argument);
static void alarm_buzzer_timer_expired(void const * argument);

static void pressure_alarm_on_timer_expired(void const * argument);
static void pressure_alarm_confirm_timer_expired(void const * argument);

static void temperature_alarm_on_timer_expired(void const *argument);
static void temperature_alarm_confirm_timer_expired(void const *argument);

static void pressure_alarm_on_timer_start(void);
static void pressure_alarm_on_timer_stop(void);

static void pressure_alarm_confirm_timer_start(void);
static void pressure_alarm_confirm_timer_stop(void);


static void temperature_alarm_on_timer_start(void);
static void temperature_alarm_on_timer_stop(void);

static void temperature_alarm_confirm_timer_start(void);
static void temperature_alarm_confirm_timer_stop(void);

static void alarm_task_req_pressure(void);
static void alarm_task_req_temperature(void);

static void alarm_buzzer_pwr_turn_on();
static void alarm_buzzer_pwr_turn_off();




static void alarm_buzzer_pwr_turn_on()
{
  bsp_buzzer_ctrl_on();
}

static void alarm_buzzer_pwr_turn_off()
{
  bsp_buzzer_ctrl_off();
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
    asw_msg.type = ALARM_SW_LONG_PRESS;
    osMessagePut(alarm_task_msg_q_id,(uint32_t)&asw_msg,0); 
    }
    }
  }else if(ptr_alarm->sw.status !=SW_STATUS_RELEASE){
   ptr_alarm->sw.status =  SW_STATUS_RELEASE;
   /*当松开按键时 如果pree_time大于短按时间且小于长按时间，认为是短按.大于长按的时间忽略*/
   if(ptr_alarm->sw.press_time >= ALARM_TASK_SW_SHORT_PRESS_TIMEOUT && ptr_alarm->sw.press_time < ALARM_TASK_SW_LONG_PRESS_TIMEOUT){
    log_debug("确认一次短按键.\r\n");
    asw_msg.type = ALARM_SW_SHORT_PRESS;
    osMessagePut(alarm_task_msg_q_id,(uint32_t)&asw_msg,0);  
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
   /*蜂鸣器状态处理*/
  if(ptr_alarm->p_status == ALARM_ON_WAIT_CLEAR      ||\
     ptr_alarm->p_status == ALARM_CONFIRM_WAIT_CLEAR ||\
     ptr_alarm->t_status == ALARM_ON_WAIT_CLEAR      ||\
     ptr_alarm->t_status == ALARM_CONFIRM_WAIT_CLEAR  ){
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

static void pressure_alarm_on_timer_init()
{
 osTimerDef(pressure_alarm_on_timer,pressure_alarm_on_timer_expired);
 pressure_alarm_on_timer_id=osTimerCreate(osTimer(pressure_alarm_on_timer),osTimerOnce,0);
 log_assert(pressure_alarm_on_timer_id);
}

static void pressure_alarm_on_timer_start(void)
{
 osTimerStart(pressure_alarm_on_timer_id,ALARM_TASK_PRESSURE_ALARM_ON_TIMEOUT);  
 log_debug("启动压力报警on定时器.\r\n");
}
static void pressure_alarm_on_timer_stop(void)
{
 osTimerStop(pressure_alarm_on_timer_id);  
 log_debug("关闭压力报警on定时器.\r\n");
}

static void pressure_alarm_on_timer_expired(void const *argument)
{
 /*首先避免竟态，获取mutex*/
 osMutexWait(alarm.mutex,ALARM_TASK_MUTEX_WAIT_TIMEOUT);
 if(ptr_alarm->p_status == ALARM_ON_WAIT_CLEAR){
 ptr_alarm->p_status = ALARM_CLEAR_WAIT_TIMEOUT;
 pressure_alarm_confirm_timer_start();
 }   
 /*释放mutex*/ 
 osMutexRelease(alarm.mutex);
}

static void pressure_alarm_confirm_timer_init()
{
 osTimerDef(pressure_alarm_confirm_timer,pressure_alarm_confirm_timer_expired);
 pressure_alarm_confirm_timer_id=osTimerCreate(osTimer(pressure_alarm_confirm_timer),osTimerOnce,0);
 log_assert(pressure_alarm_confirm_timer_id);
}

static void pressure_alarm_confirm_timer_start(void)
{
 osTimerStart(pressure_alarm_confirm_timer_id,ALARM_TASK_PRESSURE_ALARM_CONFIRM_TIMEOUT);  
 log_debug("启动压力报警confirm定时器.\r\n");
}
static void pressure_alarm_confirm_timer_stop(void)
{
 osTimerStop(pressure_alarm_confirm_timer_id);  
 log_debug("关闭压力报警confirm定时器.\r\n");
}

static void pressure_alarm_confirm_timer_expired(void const *argument)
{
 /*首先避免竟态，获取mutex*/
 osMutexWait(alarm.mutex,ALARM_TASK_MUTEX_WAIT_TIMEOUT);
 
  if(ptr_alarm->p_status == ALARM_CLEAR_WAIT_TIMEOUT){
  ptr_alarm->p_status = ALARM_TIMEOUT_WAIT_CONFIRM ;
  alarm_task_req_pressure();
 }  
 
 /*释放mutex*/ 
 osMutexRelease(alarm.mutex); 
  
}

static void alarm_task_req_pressure(void)
{
  osStatus status;
  p_msg.type = REQ_PRESSURE_VALUE;
  p_msg.req_q_id = alarm_task_msg_q_id;

 status = osMessagePut(pressure_task_msg_q_id,(uint32_t)&p_msg,ALARM_TASK_PUT_MSG_TIMEOUT);
 if(status !=osOK){
 log_error("put alarm req p msg error:%d\r\n",status); 
 }
}

static void temperature_alarm_on_timer_init()
{
 osTimerDef(temperature_alarm_on_timer,temperature_alarm_on_timer_expired);
 temperature_alarm_on_timer_id=osTimerCreate(osTimer(temperature_alarm_on_timer),osTimerOnce,0);
 log_assert(temperature_alarm_on_timer_id);
}
static void temperature_alarm_on_timer_start(void)
{
 osTimerStart(temperature_alarm_on_timer_id,ALARM_TASK_TEMPERATURE_ALARM_ON_TIMEOUT);  
 log_debug("启动温度报警on定时器.\r\n");
}

static void temperature_alarm_on_timer_stop(void)
{
 osTimerStop(temperature_alarm_on_timer_id);  
 log_debug("关闭温度报警on定时器.\r\n");
}

static void temperature_alarm_on_timer_expired(void const *argument)
{
 /*首先避免竟态，获取mutex*/
 osMutexWait(alarm.mutex,ALARM_TASK_MUTEX_WAIT_TIMEOUT);
 
 if(ptr_alarm->t_status == ALARM_ON_WAIT_CLEAR){
   ptr_alarm->t_status = ALARM_CLEAR_WAIT_TIMEOUT;
   temperature_alarm_confirm_timer_start();
 }

 /*释放mutex*/ 
 osMutexRelease(alarm.mutex); 
  
}

static void temperature_alarm_confirm_timer_init()
{
 osTimerDef(temperature_alarm_confirm_timer,temperature_alarm_confirm_timer_expired);
 temperature_alarm_confirm_timer_id=osTimerCreate(osTimer(temperature_alarm_confirm_timer),osTimerOnce,0);
 log_assert(temperature_alarm_confirm_timer_id);
}

static void temperature_alarm_confirm_timer_start(void)
{
 osTimerStart(temperature_alarm_confirm_timer_id,ALARM_TASK_TEMPERATURE_ALARM_CONFIRM_TIMEOUT);
 log_debug("启动温度报警confirm定时器.\r\n");
}
static void temperature_alarm_confirm_timer_stop()
{
 osTimerStop(temperature_alarm_confirm_timer_id);
 log_debug("关闭温度报警confirm定时器.\r\n");
}

static void temperature_alarm_confirm_timer_expired(void const *argument)
{
 /*首先避免竟态，获取mutex*/
 osMutexWait(alarm.mutex,ALARM_TASK_MUTEX_WAIT_TIMEOUT);
 
 if(ptr_alarm->t_status == ALARM_CLEAR_WAIT_TIMEOUT){
  ptr_alarm->t_status = ALARM_TIMEOUT_WAIT_CONFIRM;
  alarm_task_req_temperature();
 }
 
 /*释放mutex*/ 
 osMutexRelease(alarm.mutex);
}
static void alarm_task_req_temperature(void)
{
  osStatus status;
  t_msg.type = REQ_TEMPERATURE_VALUE;
  t_msg.req_q_id = alarm_task_msg_q_id;

 status = osMessagePut(temperature_task_msg_q_id,(uint32_t)&t_msg,ALARM_TASK_PUT_MSG_TIMEOUT);
 if(status !=osOK){
 log_error("put alarm req t msg error:%d\r\n",status); 
 }
}


void alarm_task(void const *argument)
{
  uint8_t  pressure;
  int16_t  temperature;
  osEvent  os_msg;

 
   /*定义压缩机互斥体*/
  osMutexDef(alarm_mutex);
  alarm.mutex = osMutexCreate(osMutex(alarm_mutex));
  log_assert(alarm.mutex);
  
  osMessageQDef(alarm_msg_q,6,uint32_t);
  alarm_task_msg_q_id = osMessageCreate(osMessageQ(alarm_msg_q),alarm_task_hdl);
  log_assert(alarm_task_msg_q_id);
 
  /*等待任务同步*/
  xEventGroupSync(tasks_sync_evt_group_hdl,TASKS_SYNC_EVENT_ALARM_TASK_RDY,TASKS_SYNC_EVENT_ALL_TASKS_RDY,osWaitForever);
  log_debug("alarm task sync ok.\r\n");
  
  pressure_alarm_on_timer_init();
  temperature_alarm_on_timer_init();
  pressure_alarm_confirm_timer_init();
  temperature_alarm_confirm_timer_init();
  alarm_switch_timer_init();
  alarm_switch_timer_start();
  alarm_buzzer_timer_init();
  alarm_buzzer_timer_start();
  
  /*释放mutex*/ 
  osMutexRelease(alarm.mutex);
  
  while(1){ 
  os_msg = osMessageGet(alarm_task_msg_q_id,ALARM_TASK_MSG_WAIT_TIMEOUT); 
  if(os_msg.status == osEventMessage){
  /*首先避免竟态，获取mutex*/
  osMutexWait(alarm.mutex,ALARM_TASK_MUTEX_WAIT_TIMEOUT);
  ptr_msg = (task_msg_t *)os_msg.value.v;
  
  /*广播压力值和主动请求的压力值处理*/ 
  if(ptr_msg->type == BROADCAST_PRESSURE_VALUE || ptr_msg->type == RESPONSE_PRESSURE_VALUE){
    pressure=ptr_msg->pressure;
    if(pressure == PRESSURE_ERR_VALUE_OVER_HIGH ||\
       pressure == PRESSURE_ERR_VALUE_OVER_LOW  ||\
       pressure == PRESSURE_ERR_VALUE_SENSOR    ||\
       pressure >= ALARM_TASK_PRESSURE_ALARM_VALUE){
    if(ptr_alarm->p_status == ALARM_OFF){
      ptr_alarm->p_status = ALARM_ON_WAIT_CLEAR;
      pressure_alarm_on_timer_start();
      log_warning("压力:%d kg/cm2异常，第1次报警.\r\n",pressure);
    }else if(ptr_alarm->p_status == ALARM_TIMEOUT_WAIT_CONFIRM){
      ptr_alarm->p_status = ALARM_CONFIRM_WAIT_CLEAR;
      log_warning("压力:%d kg/cm2异常，第2次报警.\r\n",pressure);
    }
    }else if(ptr_alarm->p_status != ALARM_OFF){
      ptr_alarm->p_status = ALARM_OFF;
      pressure_alarm_on_timer_stop();
      pressure_alarm_confirm_timer_stop();
      log_warning("压力%d kg/cm2恢复正常，关闭报警.\r\n",pressure);
    } 
   }
 
  /*广播温度值和主动请求的温度值处理*/ 
   if(ptr_msg->type == RESPONSE_TEMPERATURE_VALUE || ptr_msg->type == BROADCAST_TEMPERATURE_VALUE){
   temperature=ptr_msg->temperature;
   if(temperature == TEMPERATURE_ERR_VALUE_SENSOR    ||\
      temperature == TEMPERATURE_ERR_VALUE_OVER_HIGH ||\
      temperature == TEMPERATURE_ERR_VALUE_OVER_LOW     ){
    if(ptr_alarm->t_status == ALARM_OFF){
    ptr_alarm->t_status = ALARM_ON_WAIT_CLEAR;
    temperature_alarm_on_timer_start();
    log_warning("温度:%d C异常，第1次报警.\r\n",temperature);
    }else if(ptr_alarm->t_status == ALARM_TIMEOUT_WAIT_CONFIRM){
     ptr_alarm->t_status = ALARM_CONFIRM_WAIT_CLEAR;
    log_warning("温度:%d C异常，第2次报警.\r\n",temperature);
    }
   }else if(ptr_alarm->t_status != ALARM_OFF){
    ptr_alarm->t_status = ALARM_OFF; 
    temperature_alarm_on_timer_stop();
    temperature_alarm_confirm_timer_stop();
    log_warning("温度%d C恢复正常，关闭报警.\r\n",temperature);
   }
  }
  
  /*短按键消息处理*/
  if(ptr_msg->type == ALARM_SW_SHORT_PRESS){
  if(ptr_alarm->p_status == ALARM_ON_WAIT_CLEAR ||\
     ptr_alarm->t_status == ALARM_ON_WAIT_CLEAR){
   
  if(ptr_alarm->p_status == ALARM_ON_WAIT_CLEAR){
     log_warning("短按.手动解除压力第1次报警.\r\n");
     ptr_alarm->p_status = ALARM_CLEAR_WAIT_TIMEOUT; 
     pressure_alarm_on_timer_stop();
     pressure_alarm_confirm_timer_start();
  }
  if(ptr_alarm->t_status == ALARM_ON_WAIT_CLEAR){
     log_warning("短按.手动解除温度第1次报警.\r\n");
     ptr_alarm->t_status = ALARM_CLEAR_WAIT_TIMEOUT; 
     temperature_alarm_on_timer_stop();
     temperature_alarm_confirm_timer_start();
    }
    }else{
     log_debug("短按.无效.\r\n");
    }
   }
  
  
  /*长按键消息处理*/
  if(ptr_msg->type == ALARM_SW_LONG_PRESS){
  if(ptr_alarm->p_status == ALARM_ON_WAIT_CLEAR      ||\
     ptr_alarm->p_status == ALARM_CONFIRM_WAIT_CLEAR ||\
     ptr_alarm->t_status == ALARM_ON_WAIT_CLEAR      ||\
     ptr_alarm->t_status == ALARM_CONFIRM_WAIT_CLEAR)  {
      
    if(ptr_alarm->p_status == ALARM_ON_WAIT_CLEAR){
      ptr_alarm->p_status = ALARM_CLEAR_WAIT_OFF;
      pressure_alarm_on_timer_stop();
      log_warning("长按.一步彻底解除压力报警.\r\n");
    }else if( ptr_alarm->p_status == ALARM_CONFIRM_WAIT_CLEAR ){
    ptr_alarm->p_status = ALARM_CLEAR_WAIT_OFF; 
    log_warning("长按.解除压力第2次报警.\r\n");
    }
    
    if(ptr_alarm->t_status == ALARM_ON_WAIT_CLEAR){
     ptr_alarm->t_status = ALARM_CLEAR_WAIT_OFF;
     temperature_alarm_on_timer_stop();
     log_warning("长按.一步彻底解除温度报警.\r\n");  
    }else if(ptr_alarm->t_status == ALARM_CONFIRM_WAIT_CLEAR){
    ptr_alarm->t_status = ALARM_CLEAR_WAIT_OFF; 
    log_warning("长按.解除温度第2次报警.\r\n");
    }  
    }else{
    log_debug("长按.无效.\r\n");
   }
  } 
  /*释放mutex*/ 
  osMutexRelease(alarm.mutex);
  }
  }
}