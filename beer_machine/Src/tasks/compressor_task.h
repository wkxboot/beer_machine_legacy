#ifndef  __COMPRESSOR_TASK_H__
#define  __COMPRESSOR_TASK_H__
#include "stdint.h"


#ifdef  __cplusplus
#define COMPRESSOR_TASK_BEGIN  extern "C" {
#define COMPRESSOR_TASK_END    }
#else
#define COMPRESSOR_TASK_BEGIN  
#define COMPRESSOR_TASK_END   
#endif


COMPRESSOR_TASK_BEGIN

typedef enum
{
TEMPERATURE_VALUE=0
}compressor_msg_type_t;


typedef struct
{
compressor_msg_type_t type;
int16_t               value;
}compressor_msg_t;



extern osThreadId   compressor_task_hdl;
extern osMessageQId compressor_task_msg_q_id;
extern osTimerId    compressor_timer_id;


#define  COMPRESSOR_WORK_TEMPERATURE           5 /*开压缩机温度单位:摄氏度*/
#define  COMPRESSOR_STOP_TEMPERATURE           1 /*关压缩机温度单位:摄氏度*/

#define  COMPRESSOR_TASK_WORK_TIMEOUT          (120*60*1000) /*连续工作时间单位:ms*/
#define  COMPRESSOR_TASK_REST_TIMEOUT          (10*60*1000)  /*连续工作时间后的休息时间单位:ms*/
#define  COMPRESSOR_TASK_WAIT_TIMEOUT          (5*60*1000)   /*2次开机的等待时间 单位:ms*/

#define  COMPRESSOR_TASK_MSG_WAIT_TIMEOUT      osWaitForever
#define  COMPRESSOT_TASK_WAIT_RDY_TIMEOUT      (5*1000)      /*压缩机上电后等待就绪的时间*/




COMPRESSOR_TASK_END

#endif





