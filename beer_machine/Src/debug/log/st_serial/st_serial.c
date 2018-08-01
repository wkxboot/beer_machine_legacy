#include "cmsis_os.h"
#include "serial.h"
#include "usart.h"
#include "log.h"
#define  LOG_MODULE_LEVEL    LOG_LEVEL_DEBUG
#define  LOG_MODULE_NAME     "[st_serial]"

int st_serial_init(uint8_t port,uint32_t bauds,uint8_t data_bit,uint8_t stop_bit);
int st_serial_deinit(uint8_t port);
void st_serial_txe_int_enable();
void st_serial_txe_int_disable();
void st_serial_rxne_int_enable();
void st_serial_rxne_int_disable();



static serial_hal_driver_t driver={
.init=st_serial_init,
.deinit=st_serial_deinit,
.txe_int_enable=st_serial_txe_int_enable,
.txe_int_disable=st_serial_txe_int_disable,
.rxne_int_enable=st_serial_rxne_int_enable,
.rxne_int_disable=st_serial_rxne_int_disable
};

extern UART_HandleTypeDef huart1;
int    st_serial_handle;
UART_HandleTypeDef *st_serial;

  
void log_serial_init()
{
st_serial = &huart1;

serial_create(&st_serial_handle);
serial_register_hal_driver(st_serial_handle,&driver);
serial_open(st_serial_handle,1,115200,8,1);

}

void log_serial_write_byte(int ch)
{
 serial_write(st_serial_handle,(uint8_t *)&ch,1);
}



int st_serial_init(uint8_t port,uint32_t bauds,uint8_t data_bit,uint8_t stop_bit)
{
  return 0;
}

int st_serial_deinit(uint8_t port)
{
  return 0;
}
void st_serial_txe_int_enable()
{
  /*使能发送中断*/
 __HAL_UART_ENABLE_IT(st_serial,/*UART_IT_TXE*/UART_IT_TC);   
}

void st_serial_txe_int_disable()
{
 /*禁止发送中断*/
 __HAL_UART_DISABLE_IT(st_serial, /*UART_IT_TXE*/UART_IT_TC);   
}
  
void st_serial_rxne_int_enable()
{
 /*使能接收中断*/
  __HAL_UART_ENABLE_IT(st_serial,UART_IT_RXNE);  
}

void st_serial_rxne_int_disable()
{
 /*禁止接收中断*/
 __HAL_UART_DISABLE_IT(st_serial,UART_IT_RXNE); 
}


void st_serial_isr(void)
{
  uint8_t byte;
  int result;
  uint32_t tmp_flag = 0, tmp_it_source = 0; 
  
  tmp_flag = __HAL_UART_GET_FLAG(st_serial, UART_FLAG_RXNE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(st_serial, UART_IT_RXNE);
  
  /*接收中断*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  { 
  byte = (uint8_t)(st_serial->Instance->DR & (uint8_t)0x00FF);
  isr_serial_recv(st_serial_handle,byte);
  }

  tmp_flag = __HAL_UART_GET_FLAG(st_serial, /*UART_FLAG_TXE*/UART_FLAG_TC);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(st_serial, /*UART_IT_TXE*/UART_IT_TC);
  
  /*发送中断*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
   /*place this func in serial isr*/
    result =isr_serial_send( st_serial_handle,&byte);
    if(result == 0)
    {
    st_serial->Instance->DR = byte;
    }
  }  
}

uint32_t log_time()
{
  return osKernelSysTick();
}