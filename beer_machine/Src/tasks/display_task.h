#ifndef  __DISPLAY_TASK_H__
#define  __DISPLAY_TASK_H__
#include "stdint.h"


#ifdef  __cplusplus
#define DISPLAY_TASK_BEGIN  extern "C" {
#define DISPLAY_TASK_END    }
#else
#define DISPLAY_TASK_BEGIN  
#define DISPLAY_TASK_END   
#endif


DISPLAY_TASK_BEGIN

extern osThreadId   display_task_hdl;
extern osMessageQId display_task_msg_q_id;
void display_task(void const *argument);


#define  DISPLAY_TASK_MSG_WAIT_TIMEOUT         osWaitForever










DISPLAY_TASK_END


#endif