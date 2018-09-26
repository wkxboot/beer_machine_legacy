#ifndef  __PRESSURE_TASK_H__
#define  __PRESSURE_TASK_H__
#include "stdint.h"


#ifdef  __cplusplus
#define PRESSURE_TASK_BEGIN  extern "C" {
#define PRESSURE_TASK_END    }
#else
#define PRESSURE_TASK_BEGIN  
#define PRESSURE_TASK_END   
#endif


PRESSURE_TASK_BEGIN

extern osThreadId   pressure_task_hdl;
extern osMessageQId pressure_task_msg_q_id;
void pressure_task(void const *argument);

#define  PRESSURE_VALUE_IN_KG_CM2_ERR_MAX          70  /*最大错误压力。放大10倍 7.0kg/cm2*/
#define  PRESSURE_VALUE_IN_KG_CM2_MAX              65  /*最大显示压力。放大10倍 6.5kg/cm2*/
#define  PRESSURE_VALUE_IN_KG_CM2_MIN              -2  /*最小显示压力。放大10倍 -0.2kg/cm2*/
 
#define  PA_VALUE_PER_1KG_CM2                      98066.5 /*单位换算 1kg/cm2 == 98066.5Pa */


#define  PRESSURE_SENSOR_REFERENCE_VOLTAGE         3.3   /*压力传感器参考电压电压 单位:V*/
#define  PRESSURE_SENSOR_OUTPUT_VOLTAGE_MIN        0.50  /*压力传感器最小输出电压 单位:V*/
#define  PRESSURE_SENSOR_OUTPUT_VOLTAGE_MAX        4.5   /*压力传感器最大输出电压 单位:V*/

#define  PRESSURE_SENSOR_INPUT_PA_MIN             (0)/*压力传感器最小输入压力 单位:Pa*/
#define  PRESSURE_SENSOR_INPUT_PA_MAX             (1*1000000.0)/*压力传感器最大输入压力 单位:Pa*/



#define  PRESSURE_TASK_P_HOLD_TIME                 1000/*压力显示保持时间 单位:ms*/
#define  PRESSURE_SENSOR_ADC_VALUE_MAX             4095/*AD转换最大数值*/    



#define  PRESSURE_TASK_MSG_WAIT_TIMEOUT            osWaitForever
#define  PRESSURE_TASK_PUT_MSG_TIMEOUT             5

#define  PRESSURE_ERR_VALUE_OVER_HIGH              (0xe * 10 + 3)/*压力显示过载错误代码    e3*/
#define  PRESSURE_ERR_VALUE_OVER_LOW               (0xe * 10 + 4)/*压力显示低载错误代码    e4*/
#define  PRESSURE_ERR_VALUE_SENSOR                 (0xe * 10 + 5)/*压力显示AD传感器错误代码e5*/

PRESSURE_TASK_END

#endif