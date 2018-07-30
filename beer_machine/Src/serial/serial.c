#include "serial.h"
#include "cmsis_os.h"

typedef enum
{
SERIAL_TRUE=1,
SERIAL_FALSE=0
}serial_bool_t;


#ifndef  NULL
#define  NULL       (void *)0
#endif

#ifndef   IS_POWER_OF_TWO
#define   IS_POWER_OF_TWO(A)   (((A) != 0) && ((((A) - 1) & (A)) == 0))
#endif

#ifndef   MIN
#define   MIN(A,B)             ((A) > (B) ? (B) :(A))
#endif

#ifndef   MAX
#define   MAX(A,B)             ((A) > (B) ? (A) :(B))
#endif

#if !(IS_POWER_OF_TWO(BUFFER_SIZE_MAX))
#error "error in buffer size."
#endif


typedef struct
{
uint8_t     buffer[BUFFER_SIZE_MAX];
uint32_t    read_pos;
uint32_t    write_pos;

uint16_t    mask;
}serial_fifo_t;


typedef struct
{
int8_t              port;
serial_bool_t       complete;
serial_bool_t       full;
serial_hal_driver_t *hal;
serial_fifo_t       recv;
serial_fifo_t       send;
serial_bool_t       txe_int_enable;
serial_bool_t       rxne_int_enable;
serial_bool_t       valid;
int                 handle;
}serial_t;



static serial_t serial[SERIAL_CNT];



#define  ASSERT_NULL_POINTER(X)              \
{                                            \
 if((X) == NULL)                             \
 return -1;                                  \
}                                       
  
  
#define  ASSERT_HANDLE(X)                      \
{                                              \
 if(((serial_t *)(X))->handle != (X) ||        \
    ((serial_t *)(X))->valid != SERIAL_TRUE||  \
    ((serial_t *)(X))->port == -1)             \
 return -1;                                    \
}                                       
/*
***************** port function ****************************
*/

void SERIAL_ENTER_CRITICAL();
void SERIAL_EXIT_CRITICAL();

__weak void SERIAL_ENTER_CRITICAL()
{

}

__weak void SERIAL_EXIT_CRITICAL()
{

}

static void fifo_init(serial_fifo_t *fifo)
{
fifo->mask=BUFFER_SIZE_MAX-1;
fifo->read_pos=0;
fifo->write_pos=0;
}
static int fifo_free_len(serial_fifo_t *fifo)
{
  int len;
  SERIAL_ENTER_CRITICAL();
  len = BUFFER_SIZE_MAX - (fifo->write_pos-fifo->read_pos);
  SERIAL_EXIT_CRITICAL();  
  
  return len;
}

static int fifo_len(serial_fifo_t *fifo)
{
  int len;
  SERIAL_ENTER_CRITICAL();
  len = fifo->write_pos-fifo->read_pos;
  SERIAL_EXIT_CRITICAL();  
  
  return len;
}

static int fifo_flush(serial_fifo_t *fifo)
{
  int len;
	
  SERIAL_ENTER_CRITICAL();
  len = fifo->write_pos-fifo->read_pos;
  fifo->read_pos = fifo->write_pos;
  SERIAL_EXIT_CRITICAL();    
  
  return len;
}
static uint8_t fifo_get_byte(serial_fifo_t *fifo)
{
 uint8_t byte;
 
 byte=fifo->buffer[fifo->read_pos & fifo->mask];
 fifo->read_pos++;

 return byte;
}

static void fifo_put_byte(serial_fifo_t *fifo,uint8_t byte)
{
 fifo->buffer[fifo->write_pos & fifo->mask]= byte;
 fifo->write_pos++;
}


/*非阻塞模式读*/
int serial_read(int handle,uint8_t *buff,int len)
{
  int recv;
  int min;
  int read;
  serial_t *s;
    
  ASSERT_HANDLE(handle);
  s=(serial_t *)handle; 

  ASSERT_NULL_POINTER(buff);

  if(len == 0){
    return 0;
  }
  
  if(len < 0){
   return -1;
  }
  recv = fifo_len(&serial->recv);
  min = MIN(recv,len);
  
  for(read = 0;read < min;read++){
  *buff++=fifo_get_byte(&s->recv);
  }	
  if(s->full == SERIAL_TRUE){
  s->full = SERIAL_FALSE;
  s->rxne_int_enable = SERIAL_TRUE;
  s->hal->rxne_int_enable();
  }	 
  return read;
}

/*非阻塞模式写*/
int serial_write(int handle,uint8_t const *buff,int len)
{
  int free;
  int write;
  int min;
  serial_t *s;
  
  ASSERT_HANDLE(handle);
  ASSERT_NULL_POINTER(buff);
  
  s=(serial_t *)handle; 
  
  if(len == 0){
   return 0;
  }
  if(len < 0){
   return -1;
  }
  
  free = fifo_free_len(&s->send); 
  min = MIN(free,len);
  for(write =0 ;write < min;write++){
  fifo_put_byte(&s->send,*buff++);
  }
  if(s->complete == SERIAL_TRUE){
   s->complete = SERIAL_FALSE;
   s->txe_int_enable = SERIAL_TRUE;
   s->hal->txe_int_enable();
  }
  
  return write;
}

int serial_flush(int handle)
{
 int len;
 serial_t *s;
 
 ASSERT_HANDLE(handle);
 s=(serial_t *)handle; 	

 s->complete = SERIAL_TRUE;
 s->full = SERIAL_FALSE;
 fifo_flush(&s->send);
 len = fifo_flush(&s->recv);

 return len;
}

int serial_open(int handle,uint8_t port,uint32_t bauds,uint8_t data_bit,uint8_t stop_bit)
{
 int status;
 serial_t *s;
 
 ASSERT_HANDLE(handle);
 s=(serial_t *)handle;	   

 status=serial->hal->init(port,bauds,data_bit,stop_bit);
 if(status == -1){
  return -1;
 }
 fifo_init(&s->recv);
 fifo_init(&s->send);

 s->port = port;
 s->complete=SERIAL_TRUE;
 s->full = SERIAL_FALSE;
 s->rxne_int_enable=SERIAL_TRUE;
 s->hal->rxne_int_enable();

 return 0;
}


int serial_close(int handle)
{
 int status = 0;
 serial_t *s;
 
 ASSERT_HANDLE(handle);
 
 s=(serial_t *)handle;	   
 status=s->hal->deinit(s->port);
 s->port = -1;
 s->rxne_int_enable=SERIAL_FALSE;
 s->hal->rxne_int_disable();
 s->txe_int_enable=SERIAL_FALSE;
 s->hal->txe_int_disable();
 
 return status;
}

/*阻塞模式等待接收*/
int serial_select(int handle,uint32_t timeout)
{
 int len;
 serial_t *s;
 
 ASSERT_HANDLE(handle);
 
 s=(serial_t *)handle;	  

 while(1){
 len=fifo_len(&s->recv);
 if(len == 0 && timeout > 0){
  osDelay(1);
  timeout--;
 }else{
  break;
 }
 }
 
 return len;
}
/*阻塞模式等待发送*/
int serial_complete(int handle,uint32_t timeout)
{
 int len;
 serial_t *s;
 ASSERT_HANDLE(handle);
 s =(serial_t *)handle;	  
 ASSERT_NULL_POINTER(serial);

 while(s->complete == SERIAL_FALSE && timeout-- >0){
 osDelay(1);
 }

 len=fifo_len(&s->send);
 return len;
}

int serial_register_hal_driver(int handle,serial_hal_driver_t *hal)
{
  serial_t *s;
  
  ASSERT_HANDLE(handle);
  
  s=(serial_t *)handle; 

  ASSERT_NULL_POINTER(hal);
  ASSERT_NULL_POINTER(hal->init);
  ASSERT_NULL_POINTER(hal->deinit);
  ASSERT_NULL_POINTER(hal->txe_int_enable);
  ASSERT_NULL_POINTER(hal->txe_int_disable);
  ASSERT_NULL_POINTER(hal->rxne_int_enable);
  ASSERT_NULL_POINTER(hal->rxne_int_disable);
  s->hal = hal;
  return 0;
}



/*place this func in serial isr*/
int isr_serial_send(int handle,uint8_t *byte)
{
  int len;
  serial_t *s;
  
  ASSERT_HANDLE(handle);
  ASSERT_NULL_POINTER(byte);
  
  s=(serial_t *)handle;	 

  len =fifo_len(&s->send);
  if(len == 0){
  s->complete = SERIAL_TRUE;
  s->txe_int_enable = SERIAL_FALSE;
  s->hal->txe_int_disable();
  return -1;
  }
  
  *byte = fifo_get_byte(&s->send);
  return 0;
}

int isr_serial_recv(int handle,uint8_t byte)
{
 
 int len;
 serial_t *s;
 
 ASSERT_HANDLE(handle);
 s=(serial_t *)handle;	   

 len =fifo_free_len(&s->recv);
 if(len == 0){
 s->full = SERIAL_TRUE;
 s->rxne_int_enable = SERIAL_FALSE;
 s->hal->rxne_int_disable();
 return -1;
 }
 fifo_put_byte(&s->recv,byte);
 
 return 0;
}

int serial_create(int *handle)
{
 uint8_t i;
 
 ASSERT_NULL_POINTER(handle);

 for(i=0;i< SERIAL_CNT;i++){
 	if(serial[i].valid == SERIAL_FALSE){
	   serial[i].valid= SERIAL_TRUE;
       serial[i].handle =(int)&serial[i];
	   *handle = (int)&serial[i];
       
	   return 0;
 		}
 	}

 return -1;		
}

int serial_destroy(int handle)
{
 serial_t *s;
 
 ASSERT_HANDLE(handle);

 s=( serial_t *)handle;
 s->valid=SERIAL_FALSE;
 s->handle =0;

 return 0;
}


