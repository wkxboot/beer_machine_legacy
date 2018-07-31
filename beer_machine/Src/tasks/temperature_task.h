#ifndef  __TEMPERATURE_TASK_H__
#define  __TEMPERATURE_TASK_H__
#include "stdint.h"


#ifdef  __cplusplus
#define TEMPERATURE_TASK_BEGIN  extern "C" {
#define TEMPERATURE_TASK_END    }
#else
#define TEMPERATURE_TASK_BEGIN  
#define TEMPERATURE_TASK_END   
#endif


TEMPERATURE_TASK_BEGIN

extern osThreadId   temperature_task_hdl;
extern osMessageQId temperature_task_msg_q_id;


typedef enum
{
T_ADC_COMPLETED=0,
REQ_TEMPERATURE
}temperature_msg_type_t;


typedef struct
{
temperature_msg_type_t type;
uint16_t               value;
}temperature_msg_t;



#define  TEMPERATURE_TASK_T_HOLD_TIME              8000/*温度显示保持时间 单位:ms*/


#define  TEMPERATURE_SENSOR_ADC_VALUE_MAX          4095/*温度AD转换最大数值*/      
#define  TEMPERATURE_SENSOR_BYPASS_RES_VALUE       5100/*温度AD转换旁路电阻值*/  
#define  TEMPERATURE_SENSOR_REFERENCE_VOLTAGE      3.3 /*温度传感器参考电压 单位:V*/
#define  TEMPERATURE_SENSOR_SUPPLY_VOLTAGE         5.0 /*温度传感器供电电压 单位:V*/


#define  TEMPERATURE_TASK_MSG_WAIT_TIMEOUT         osWaitForever

#define  TEMPERATURE_ERR_VALUE_OVER_HIGH           0xe0/*温度显示过载错误代码*/
#define  TEMPERATURE_ERR_VALUE_OVER_LOW            0xe1/*温度显示低载错误代码*/
#define  TEMPERATURE_ERR_VALUE_SENSOR              0xe3/*温度显示传感器错误代码*/

TEMPERATURE_TASK_END

#endif