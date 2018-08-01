#include "cmsis_os.h"   
#include "board.h"
#include "fsl_usart.h"
#include "fsl_clock.h"
#include "pin_mux.h"

#include "modbus.h"

/*lpc824在IAR freertos下的移植*/
int uart_init(uint8_t port,uint32_t bauds,uint8_t data_bit,uint8_t stop_bit);
void serial_txe_int_enable();
void serial_txe_int_disable();
void serial_rxne_int_enable();
void serial_rxne_int_disable();

serial_hal_driver_t hal_driver={
.init =uart_init,
.txe_int_enable=serial_txe_int_enable,
.txe_int_disable =serial_txe_int_disable,
.rxne_int_enable =serial_rxne_int_enable,
.rxne_int_disable =serial_rxne_int_disable
};

extern modbus_t *ctx;
USART_Type *serial;
IRQn_Type  serial_irq_num;


int uart_init(uint8_t port,uint32_t bauds,uint8_t data_bit,uint8_t stop_bit)
{
    status_t status;
    usart_config_t config;

       
    if(port == 0){
    serial=USART0;
    serial_irq_num=USART0_IRQn;
    }else if(port == 1){
    serial=USART1;
    serial_irq_num=USART2_IRQn;
    }else if(port == 2){
    serial=USART2;
    serial_irq_num=USART2_IRQn;
    }else{
    serial=USART0;
    serial_irq_num=USART0_IRQn;
    }
    
    config.baudRate_Bps = bauds;
    if(data_bit == 8){
    config.bitCountPerChar = kUSART_8BitsPerChar;
    }else{
    config.bitCountPerChar = kUSART_7BitsPerChar;
    }
    if(stop_bit ==1){
    config.stopBitCount = kUSART_OneStopBit;
    }else{
    config.stopBitCount = kUSART_TwoStopBit;
    }
    
    config.parityMode = kUSART_ParityDisabled;
    config.syncMode = kUSART_SyncModeDisabled;
    
    config.loopback = false;
    config.enableRx = true;
    config.enableTx = true;
    /* Initialize the USART with configuration. */
    status=USART_Init(serial, &config, CLOCK_GetFreq(kCLOCK_MainClk));
 
  if (status != 0){
    return -1;
  } 
  EnableIRQ(serial_irq_num);
  return 0;
}

void serial_txe_int_enable()
{
 USART_EnableInterrupts(serial,USART_INTENSET_TXRDYEN_MASK);
}
void serial_txe_int_disable()
{
  USART_DisableInterrupts(serial,USART_INTENSET_TXRDYEN_MASK);  
}

void serial_rxne_int_enable()
{
 USART_EnableInterrupts(serial,USART_INTENSET_RXRDYEN_MASK);
}
void serial_rxne_int_disable()
{
 USART_DisableInterrupts(serial,USART_INTENSET_RXRDYEN_MASK);
}


void serial_isr()
{
  uint32_t tmp_flag = 0, tmp_it_source = 0; 
  uint8_t  send_byte,recv_byte;
  
  tmp_flag = USART_GetEnabledInterrupts(serial);
  tmp_it_source =USART_GetStatusFlags(serial);
  
 /*接收中断处理*/
  if((tmp_flag & USART_INTENSET_RXRDYEN_MASK) && (tmp_it_source & USART_STAT_RXRDY_MASK)){
      recv_byte=USART_ReadByte(serial);
      isr_serial_recv(ctx->s,recv_byte);
  }
 /*发送中断处理*/
  if((tmp_flag & USART_INTENSET_TXRDYEN_MASK) && (tmp_it_source & USART_STAT_TXRDY_MASK)){
      if(isr_serial_send(ctx->s,&send_byte)!= -1){    
      USART_WriteByte(serial, send_byte);
      }
   }
}
/*串口中断处理*/
void USART0_IRQHandler()
{
  serial_isr();
}
void USART1_IRQHandler()
{
  serial_isr();
}
void USART2_IRQHandler()
{
  serial_isr();
}