#include "cmsis_os.h"
#include "tasks_init.h"
#include "compressor_task.h"
#include "temperature_task.h"
#include "beer_machine.h"
#include "log.h"
#define  LOG_MODULE_LEVEL    LOG_LEVEL_DEBUG
#define  LOG_MODULE_NAME     "[c_task]"

osThreadId   compressor_task_hdl;
osMessageQId compressor_task_msg_q_id;
osTimerId    compressor_timer_id;

static task_msg_t  t_msg;
static task_msg_t  c_msg;
static task_msg_t  *ptr_msg;

typedef enum
{
COMPRESSOR_STATUS_RDY=0, /*就绪状态*/
COMPRESSOR_STATUS_REST,  /*长时间工作后停机的状态*/
COMPRESSOR_STATUS_WORK,  /*压缩机工作状态*/
COMPRESSOR_STATUS_WAIT   /*两次开机之间的状态*/
}compressor_status_t;

static compressor_status_t compressor_status = COMPRESSOR_STATUS_RDY;

static void compressor_timer_expired(void const * argument);
static void compressor_pwr_turn_on();
static void compressor_pwr_turn_off();



static void compressor_timer_init()
{
 osTimerDef(compressor_timer,compressor_timer_expired);
 compressor_timer_id=osTimerCreate(osTimer(compressor_timer),osTimerOnce,0);
 log_assert(compressor_timer_id);
}

static void compressor_timer_start_work_timer(void)
{
 osTimerStart(compressor_timer_id,COMPRESSOR_TASK_WORK_TIMEOUT);  
 log_debug("压缩机工作定时器开始.\r\n");
}
static void compressor_timer_start_rest_timer(void)
{
 osTimerStart(compressor_timer_id,COMPRESSOR_TASK_REST_TIMEOUT);
 log_debug("压缩机休息定时器开始.\r\n");
}

static void compressor_timer_start_wait_timer(void)
{
 osTimerStart(compressor_timer_id,COMPRESSOR_TASK_WAIT_TIMEOUT);
 log_debug("压缩机等待定时器开始.\r\n");
}

static void compressor_timer_stop()
{
 osTimerStop(compressor_timer_id);
 log_debug("压缩机定时器停止.\r\n");
}
static void compressor_timer_expired(void const *argument)
{
  osStatus status;
  c_msg.type = COMPRESSOR_TIMER_EXPIRED;
  status = osMessagePut(compressor_task_msg_q_id,(uint32_t)&c_msg,0);
  if(status !=osOK){
  log_error("put compressor msg error:%d\r\n",status);
  }
  
}


static void compressor_pwr_turn_on()
{
 bsp_compressor_ctrl_on(); 
}
static void compressor_pwr_turn_off()
{
 bsp_compressor_ctrl_off();  
}

void compressor_task(void const *argument)
{
  int16_t t;
  osEvent os_msg;
  osStatus status;
  
  osMessageQDef(compressor_msg_q,4,uint32_t);
  compressor_task_msg_q_id = osMessageCreate(osMessageQ(compressor_msg_q),compressor_task_hdl);
  log_assert(compressor_task_msg_q_id);
  /*开机先关闭压缩机*/
  compressor_pwr_turn_off();
  /*等待任务同步*/
  xEventGroupSync(tasks_sync_evt_group_hdl,TASKS_SYNC_EVENT_COMPRESSOR_TASK_RDY,TASKS_SYNC_EVENT_ALL_TASKS_RDY,osWaitForever);
  log_debug("compressor task sync ok.\r\n");
  
  compressor_timer_init();
  osDelay(COMPRESSOT_TASK_WAIT_RDY_TIMEOUT);
  
  while(1){
  os_msg = osMessageGet(compressor_task_msg_q_id,COMPRESSOR_TASK_MSG_WAIT_TIMEOUT);
  if(os_msg.status == osEventMessage){  
   ptr_msg =(task_msg_t*)os_msg.value.v;
   /*温度消息处理*/
  if(ptr_msg->type == RESPONSE_TEMPERATURE_VALUE ||ptr_msg->type == BROADCAST_TEMPERATURE_VALUE){ 
  t=ptr_msg->temperature;   
  if(t == TEMPERATURE_ERR_VALUE_SENSOR    ||\
     t == TEMPERATURE_ERR_VALUE_OVER_HIGH ||\
     t == TEMPERATURE_ERR_VALUE_OVER_LOW ){
      if(compressor_status != COMPRESSOR_STATUS_RDY){
        compressor_status = COMPRESSOR_STATUS_RDY;
        compressor_timer_stop();
        compressor_pwr_turn_off();
        log_error("温度错误.关压缩机.code:%d\r\n",t);
       }
  }else{
    if(t >= COMPRESSOR_WORK_TEMPERATURE && compressor_status == COMPRESSOR_STATUS_RDY){
     compressor_pwr_turn_on();
     compressor_status = COMPRESSOR_STATUS_WORK;
     compressor_timer_start_work_timer();
     log_debug("温度高于开机温度.开压缩机.\r\n");
    }else if(t <= COMPRESSOR_STOP_TEMPERATURE && compressor_status == COMPRESSOR_STATUS_WORK){
     compressor_pwr_turn_off();
     compressor_status = COMPRESSOR_STATUS_WAIT;
     compressor_timer_start_wait_timer();
     log_debug("温度低于关机温度.关压缩机.\r\n");
    }
  } 
 }
 
 /*定时器时间到达处理处理*/
 if(ptr_msg->type == COMPRESSOR_TIMER_EXPIRED){
   if(compressor_status == COMPRESSOR_STATUS_WORK){
   compressor_pwr_turn_off();
   compressor_status = COMPRESSOR_STATUS_REST;
   compressor_timer_start_rest_timer();
   log_debug("压缩机到达最长工作时间.关压缩机\r\n");
   }else{
   compressor_status = COMPRESSOR_STATUS_RDY;
   /*需要请求当前温度值，以判断是否需要控制压缩机*/
   log_debug("压缩机定时器到达.请求温度.\r\n");
   t_msg.type=REQ_TEMPERATURE_VALUE; 
   t_msg.req_q_id = compressor_task_msg_q_id;
   status = osMessagePut(temperature_task_msg_q_id,(uint32_t)&t_msg,0);
   if(status !=osOK){
   log_error("put temperature msg error:%d\r\n",status);
  }
 }    
 }
 
 
 
  }
  }
}

