#ifndef  __ALARM_TASK_H__
#define  __ALARM_TASK_H__
#include "stdint.h"


#ifdef  __cplusplus
#define ALARM_TASK_BEGIN  extern "C" {
#define ALARM_TASK_END    }
#else
#define ALARM_TASK_BEGIN  
#define ALARM_TASK_END   
#endif


ALARM_TASK_BEGIN

#define  ALARM_TASK_SWITCH_MONITOR_TIMEOUT             10  /*按键监视间隔时间 单位:ms*/
#define  ALARM_TASK_BUZZER_MONITOR_TIMEOUT             10  /*按键监视间隔时间 单位:ms*/
#define  ALARM_TASK_PUT_MSG_TIMEOUT                    5   /*报警任务消息发送时间 单位:ms*/

#define  ALARM_TASK_MSG_WAIT_TIMEOUT                   osWaitForever /*报警任务消息等待时间 单位:ms*/
#define  ALARM_TASK_MUTEX_WAIT_TIMEOUT                 osWaitForever /*mutex消息等待时间 单位:ms*/

#define  ALARM_TASK_SW_SHORT_PRESS_TIMEOUT             (40)    /*按键短按有效时间 单位:ms*/
#define  ALARM_TASK_SW_LONG_PRESS_TIMEOUT              (3*1000)/*按键长按有效时间 单位:ms*/


#define  ALARM_TASK_PRESSURE_ALARM_ON_TIMEOUT          (1*60*1000) /*压力第一次报警超时时间长     单位:ms*/             
#define  ALARM_TASK_PRESSURE_ALARM_CONFIRM_TIMEOUT     (10*60*1000)/*压力第二次报警等待时间长 单位:ms*/ 

#define  ALARM_TASK_TEMPERATURE_ALARM_ON_TIMEOUT       (1*60*1000) /*温度第一次报警超时时间长     单位:ms*/             
#define  ALARM_TASK_TEMPERATURE_ALARM_CONFIRM_TIMEOUT  (10*60*1000)/*温度第二次报警等待时间长 单位:ms*/ 



#define  ALARM_TASK_PRESSURE_ALARM_VALUE                50 /*蜂鸣器高压报警放大10倍  单位:kg/cm2*/



#define  ALARM_TASK_ALARM_RATE_HZ                       0.5 /*蜂鸣器频率   单位:Hz*/
#define  ALARM_TASK_ALARM_DUTY_ON_RATE                  20  /*蜂鸣器占空比 单位:%*/

extern osThreadId   alarm_task_hdl;
extern osMessageQId alarm_task_msg_q_id;
void alarm_task(void const *argument);




ALARM_TASK_END

#endif