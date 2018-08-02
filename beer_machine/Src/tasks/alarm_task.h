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

#define  ALARM_TASK_SWITCH_MONITOR_TIMEOUT        10  /*按键监视间隔时间 单位:ms*/
#define  ALARM_TASK_BUZZER_MONITOR_TIMEOUT        10  /*按键监视间隔时间 单位:ms*/
#define  ALARM_TASK_MSG_SEND_TIMEOUT              5   /*报警任务消息发送时间 单位:ms*/

#define  ALARM_TASK_MSG_WAIT_TIMEOUT              osWaitForever /*报警任务消息等待时间 单位:ms*/


#define  ALARM_TASK_SW_SHORT_PRESS_TIMEOUT        (50)    /*按键短按有效时间 单位:ms*/
#define  ALARM_TASK_SW_LONG_PRESS_TIMEOUT         (3*1000)/*按键长按有效时间 单位:ms*/



#define  ALARM_TASK_PRESSURE_ALARM_VALUE          99/*报警值放大10倍       单位:kg/cm2*/

#define  ALARM_TASK_ALARM_ON_FIRST_TIMEOUT        (1*60*1000) /*第一次报警时间     单位:ms*/   
#define  ALARM_TASK_ALARM_WAIT_TIMEOUT            (10*60*1000)/*第二次报警等待时间 单位:ms*/  

#define  ALARM_TASK_ALARM_RATE_HZ                 2  /*蜂鸣器频率   单位:Hz*/
#define  ALARM_TASK_ALARM_DUTY_ON_RATE            50 /*蜂鸣器占空比 单位:%*/

extern osThreadId   alarm_task_hdl;
extern osMessageQId alarm_task_msg_q_id;
void alarm_task(void const *argument);




ALARM_TASK_END

#endif