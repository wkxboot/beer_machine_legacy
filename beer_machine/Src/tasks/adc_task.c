#include "cmsis_os.h"
#include "adc.h"
#include "tasks_init.h"
#include "adc_task.h"
#include "temperature_task.h"
#include "pressure_task.h"
#include "beer_machine.h"

#include "log.h"
#define  LOG_MODULE_LEVEL    LOG_LEVEL_DEBUG
#define  LOG_MODULE_NAME     "[adc_task]"

osThreadId   adc_task_hdl;

static uint16_t adc_sample[2];
static uint32_t adc_cusum[2];
static uint16_t adc_average[2];

static task_msg_t    t_msg;
static task_msg_t    p_msg;


void  HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
osSignalSet(adc_task_hdl,ADC_TASK_ADC_COMPLETED_SIGNAL);   
}


void  HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
 osSignalSet(adc_task_hdl,ADC_TASK_ADC_ERROR_SIGNAL);   
}


static int adc_start()
{
  HAL_StatusTypeDef status;
  status =HAL_ADC_Start_DMA(&hadc1,(uint32_t*)adc_sample,2);
  if(status != HAL_OK){
   log_error("start adc dma error:%d\r\n",status);
   return -1;
  }
  
  return 0;
}

static int adc_stop()
{
  HAL_StatusTypeDef status;
  status =HAL_ADC_Stop_DMA(&hadc1);
  if(status != HAL_OK){
   log_error("stop adc dma error:%d\r\n",status);
   return -1;
  }
  
  return 0;
}
static void adc_reset()
{
  HAL_ADC_MspDeInit(&hadc1);
  MX_ADC1_Init();
}

void adc_task(void const * argument)
{
  osEvent signals;
  osStatus status;
  uint8_t t_sample_err_cnt=0,p_sample_err_cnt=0;
  uint8_t t_sample_cnt=0    ,p_sample_cnt=0;
  int  result;
  /*等待任务同步*/
  xEventGroupSync(tasks_sync_evt_group_hdl,TASKS_SYNC_EVENT_ADC_TASK_RDY,TASKS_SYNC_EVENT_ALL_TASKS_RDY,osWaitForever);
  log_debug("adc task sync ok.\r\n");
  adc_stop();
  while(1){
  osDelay(ADC_TASK_INTERVAL);
  result = adc_start();
  if(result !=0){
    adc_reset();
    continue;
  } 
  
  signals = osSignalWait(ADC_TASK_ALL_SIGNALS,ADC_TASK_ADC_TIMEOUT);
  if(signals.status == osEventSignal ){
  if(signals.value.signals & ADC_TASK_ADC_COMPLETED_SIGNAL){
    
/*temperature adc calculate*/    
    if(t_sample_cnt < ADC_TASK_ADC_SAMPLE_MAX){
      if(adc_sample[ADC_TASK_TEMPERATURE_IDX] == ADC_TASK_ADC_VALUE_MIN ||\
         adc_sample[ADC_TASK_TEMPERATURE_IDX] == ADC_TASK_ADC_VALUE_MAX){
           
         adc_cusum[ADC_TASK_TEMPERATURE_IDX]=0;
         t_sample_cnt=0;
         t_sample_err_cnt++;
    
         if(t_sample_err_cnt >= ADC_TASK_ADC_ERR_MAX){
         log_error("temperature sample error.\r\n");
         t_msg.type=T_ADC_COMPLETED;
         t_msg.adc=ADC_TASK_ADC_ERR_VALUE;         
         status = osMessagePut(temperature_task_msg_q_id,(uint32_t)&t_msg,0);
         if(status !=osOK){
         log_error("put err temperature msg error:%d\r\n",status);
         }   
       }
     }else{
       adc_cusum[ADC_TASK_TEMPERATURE_IDX]+=adc_sample[ADC_TASK_TEMPERATURE_IDX];   
       t_sample_cnt++;
       t_sample_err_cnt=0;
     }
    }else{
     adc_average[ADC_TASK_TEMPERATURE_IDX]=adc_cusum[ADC_TASK_TEMPERATURE_IDX]/t_sample_cnt;  
     adc_cusum[ADC_TASK_TEMPERATURE_IDX]=0;
     t_sample_cnt=0;
     t_msg.type=T_ADC_COMPLETED;
     t_msg.adc=adc_average[ADC_TASK_TEMPERATURE_IDX];  
     status = osMessagePut(temperature_task_msg_q_id,(uint32_t)&t_msg,0);
     if(status !=osOK){
      log_error("put temperature msg error:%d\r\n",status);
     }
    }
    
   /*pressure adc calculate*/ 
     if(p_sample_cnt < ADC_TASK_ADC_SAMPLE_MAX){
      if(adc_sample[ADC_TASK_PRESSURE_IDX] == ADC_TASK_ADC_VALUE_MIN ||\
         adc_sample[ADC_TASK_PRESSURE_IDX] == ADC_TASK_ADC_VALUE_MAX){
           
         adc_cusum[ADC_TASK_PRESSURE_IDX]=0;
         p_sample_cnt=0;
         p_sample_err_cnt++;
         
         if(p_sample_err_cnt >= ADC_TASK_ADC_ERR_MAX){
         log_error("pressure  sample error.\r\n");
         p_msg.type=P_ADC_COMPLETED;
         p_msg.adc=ADC_TASK_ADC_ERR_VALUE; 
         status = osMessagePut(pressure_task_msg_q_id,(uint32_t)&p_msg,0);    
         if(status !=osOK){
         log_error("put err pressure msg error:%d\r\n",status);
         }   
       }
     }else{
       adc_cusum[ADC_TASK_PRESSURE_IDX]+=adc_sample[ADC_TASK_PRESSURE_IDX]; 
       p_sample_err_cnt=0;
       p_sample_cnt++;
     }
    }else{
     adc_average[ADC_TASK_PRESSURE_IDX]=adc_cusum[ADC_TASK_PRESSURE_IDX]/p_sample_cnt;  
     adc_cusum[ADC_TASK_PRESSURE_IDX]=0;
     p_sample_cnt=0;
     p_msg.type=P_ADC_COMPLETED;
     p_msg.adc=adc_average[ADC_TASK_PRESSURE_IDX];  
     status = osMessagePut(pressure_task_msg_q_id,(uint32_t)&p_msg,0);
     if(status !=osOK){
     log_error("put error pressure msg error:%d\r\n",status);
      }    
    }    
  }

  if(signals.value.signals & ADC_TASK_ADC_ERROR_SIGNAL){
  log_error("adc error.reset.\r\n");
  adc_reset();
  }
  }
  
  }
}

