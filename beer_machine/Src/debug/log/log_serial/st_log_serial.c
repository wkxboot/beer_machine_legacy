#include "cmsis_os.h"
#include "serial.h"
#include "usart.h"
#include "log.h"
#define  LOG_MODULE_LEVEL    LOG_LEVEL_DEBUG
#define  LOG_MODULE_NAME     "[st_log_serial]"

int st_log_serial_init(uint8_t port,uint32_t bauds,uint8_t data_bit,uint8_t stop_bit);
int st_log_serial_deinit(uint8_t port);
void st_log_serial_enable_txe_int();
void st_log_serial_disable_txe_int();
void st_log_serial_enable_rxne_int();
void st_log_serial_disable_rxne_int();


extern int log_serial_handle;

serial_hal_driver_t log_serial_driver={
.init=st_log_serial_init,
.deinit=st_log_serial_deinit,
.enable_txe_int=st_log_serial_enable_txe_int,
.disable_txe_int=st_log_serial_disable_txe_int,
.enable_rxne_int=st_log_serial_enable_rxne_int,
.disable_rxne_int=st_log_serial_disable_rxne_int
};

extern UART_HandleTypeDef huart1;
UART_HandleTypeDef *st_serial = &huart1;



int st_log_serial_init(uint8_t port,uint32_t bauds,uint8_t data_bit,uint8_t stop_bit)
{
 (void)port;

  //huart1.Instance = USART1;
  st_serial->Init.BaudRate = bauds;
  if(data_bit == 8){
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  }else{
  st_serial->Init.WordLength = UART_WORDLENGTH_9B;
  }
  if(stop_bit == 1){
  st_serial->Init.StopBits = UART_STOPBITS_1;
  }else{
  st_serial->Init.StopBits = UART_STOPBITS_2; 
  }
  st_serial->Init.Parity = UART_PARITY_NONE;
  st_serial->Init.Mode = UART_MODE_TX_RX;
  st_serial->Init.HwFlowCtl = UART_HWCONTROL_NONE;
  st_serial->Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(st_serial) != HAL_OK){
  return -1;
  }
  return 0;
}


int st_log_serial_deinit(uint8_t port)
{
  return 0;
}

void st_log_serial_enable_txe_int()
{
  /*使能发送中断*/
 __HAL_UART_ENABLE_IT(st_serial,/*UART_IT_TXE*/UART_IT_TC);   
}

void st_log_serial_disable_txe_int()
{
 /*禁止发送中断*/
 __HAL_UART_DISABLE_IT(st_serial, /*UART_IT_TXE*/UART_IT_TC);   
}
  
void st_log_serial_enable_rxne_int()
{
 /*使能接收中断*/
  __HAL_UART_ENABLE_IT(st_serial,UART_IT_RXNE);  
}

void st_log_serial_disable_rxne_int()
{
 /*禁止接收中断*/
 __HAL_UART_DISABLE_IT(st_serial,UART_IT_RXNE); 
}


void st_log_serial_isr(void)
{
  int result;
  uint8_t recv_byte,send_byte;
  uint32_t tmp_flag = 0, tmp_it_source = 0; 
  
  tmp_flag = __HAL_UART_GET_FLAG(st_serial, UART_FLAG_RXNE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(st_serial, UART_IT_RXNE);
  
  /*接收中断*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  { 
  recv_byte = (uint8_t)(st_serial->Instance->DR & (uint8_t)0x00FF);
  isr_serial_put_byte_from_recv(log_serial_handle,recv_byte);
  }

  tmp_flag = __HAL_UART_GET_FLAG(st_serial, /*UART_FLAG_TXE*/UART_FLAG_TC);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(st_serial, /*UART_IT_TXE*/UART_IT_TC);
  
  /*发送中断*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
    result =isr_serial_get_byte_to_send(log_serial_handle,&send_byte);
    if(result == 1)
    {
    st_serial->Instance->DR = send_byte;
    }
  }  
}