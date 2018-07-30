#include "cmsis_os.h"
#include "adc.h"
#include "adc_task.h"
#include "beer_machine.h"
#include "log.h"
#define  LOG_MODULE_LEVEL    LOG_LEVEL_DEBUG
#define  LOG_MODULE_NAME     "[adc_task]"

osThreadId   adc_task_hdl;

osMessageQId temperature_msg_q_hdl;
osMessageQId pressure_msg_q_hdl;

static uint16_t adc_sample[2];
static uint32_t adc_cusum[2];
static uint16_t adc_average[2];




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
    
/*temperature calculate*/    
    if(t_sample_cnt < ADC_TASK_ADC_SAMPLE_MAX){
      if(adc_sample[ADC_TASK_TEMPERATURE_IDX] == ADC_TASK_ADC_VALUE_MIN ||\
         adc_sample[ADC_TASK_TEMPERATURE_IDX] == ADC_TASK_ADC_VALUE_MAX){
           
         adc_cusum[ADC_TASK_TEMPERATURE_IDX]=0;
         t_sample_cnt=0;
         t_sample_err_cnt++;
    
         if(t_sample_err_cnt >= ADC_TASK_ADC_ERR_MAX){
         log_error("temperature  sample error.\r\n");
         status = osMessagePut(temperature_msg_q_hdl,ADC_TASK_ADC_ERR_VALUE,0);
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
     status = osMessagePut(temperature_msg_q_hdl,adc_average[ADC_TASK_TEMPERATURE_IDX],0);
     if(status !=osOK){
      log_error("put temperature msg error:%d\r\n",status);
     }
    }
    
   /*pressure calculate*/ 
     if(p_sample_cnt < ADC_TASK_ADC_SAMPLE_MAX){
      if(adc_sample[ADC_TASK_PRESSURE_IDX] == ADC_TASK_ADC_VALUE_MIN ||\
         adc_sample[ADC_TASK_PRESSURE_IDX] == ADC_TASK_ADC_VALUE_MAX){
           
         adc_cusum[ADC_TASK_PRESSURE_IDX]=0;
         p_sample_cnt=0;
         p_sample_err_cnt++;
         
         if(p_sample_err_cnt >= ADC_TASK_ADC_ERR_MAX){
         log_error("pressure  sample error.\r\n");
         status = osMessagePut(pressure_msg_q_hdl,ADC_TASK_ADC_ERR_VALUE,0);
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
     
     status = osMessagePut(pressure_msg_q_hdl,adc_average[ADC_TASK_PRESSURE_IDX],0);
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

