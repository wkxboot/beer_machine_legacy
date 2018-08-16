#include "serial.h"
#include "cmsis_os.h"
#include "comm_utils.h"
typedef struct
{
uint8_t     *pbuffer;
uint32_t    read;
uint32_t    write;
uint32_t    size;
}fifo_t;


typedef struct
{
int                 handle;
int8_t              port;
bool                registered;
bool                complete;
bool                full;
bool                txe_int_enable;
bool                rxne_int_enable;
bool                valid; 
serial_hal_driver_t *driver;
fifo_t              recv;
fifo_t              send;
}serial_t;


  
#define  ASSERT_HANDLE(X)                       \
{                                               \
 if(((serial_t *)(X))->handle != (X))           \
 return -1;                                     \
} 



static int fifo_free_size(fifo_t *fifo)
{
  int len;
  len = fifo->size - (fifo->write-fifo->read); 
  return len;
}

static int fifo_used_size(fifo_t *fifo)
{
  int len;
  len = fifo->write - fifo->read; 
  return len;
}

static int fifo_flush(fifo_t *fifo)
{
  int len;	
  len = fifo->write - fifo->read;
  fifo->read = fifo->write;   
  return len;
}


static int fifo_get(fifo_t *fifo,uint8_t *pbuffer,uint16_t size)
{
 uint16_t cpy_cnt,pos;

 for(cpy_cnt=0;cpy_cnt<size;cpy_cnt++){
 if(fifo_used_size(fifo) > 0){
 pos = fifo->read % fifo->size;
 *pbuffer=fifo->pbuffer[pos];
 fifo->read++;
 pbuffer++;
 }else{
 break;
 }
 }
 return cpy_cnt;
}

static int fifo_put(fifo_t *fifo,uint8_t const *pbuffer,uint16_t size)
{
 uint16_t cpy_cnt,pos;

 for(cpy_cnt=0;cpy_cnt<size;cpy_cnt++){
 if(fifo_free_size(fifo) > 0){
 pos = fifo->write % fifo->size;
 fifo->pbuffer[pos]=*pbuffer;
 fifo->write++;
 pbuffer++;
 }else{
 break;
 }
 }
 return cpy_cnt;
}


/*非阻塞模式读*/
int serial_read(int handle,uint8_t *pbuffer,int size)
{
  int read;
  serial_t *s;
    
  ASSERT_HANDLE(handle);
  s=(serial_t *)handle;
  ASSERT_NULL_POINTER(pbuffer);
  
  if(size < 0){
   return -1;
  }
  if(s->port == -1){
  return -1;
 } 
  SERIAL_ENTER_CRITICAL();
  read = fifo_get(&s->recv,pbuffer,size); 
  if(s->full == true){
  s->full = false;
  s->rxne_int_enable = true;
  s->driver->enable_rxne_int();
  }	 
  SERIAL_EXIT_CRITICAL(); 
  return read;
}

/*非阻塞模式写*/
int serial_write(int handle,uint8_t const *pbuffer,int size)
{
  int write;
  serial_t *s;
  
  ASSERT_HANDLE(handle);
  ASSERT_NULL_POINTER(pbuffer);
  s=(serial_t *)handle; 
  if(size < 0){
   return -1;
  }
  if(s->port == -1){
  return -1;
  } 
  SERIAL_ENTER_CRITICAL();
  write = fifo_put(&s->send,pbuffer,size);
  if(s->complete == true){
   s->complete = false;
   s->txe_int_enable = true;
   s->driver->enable_txe_int();
  }
  SERIAL_EXIT_CRITICAL();
  return write;
}

int serial_flush(int handle)
{
 int size;
 serial_t *s;
 
 ASSERT_HANDLE(handle);
 s=(serial_t *)handle; 	
 SERIAL_ENTER_CRITICAL();
 s->complete = true;
 s->full = false;
 fifo_flush(&s->send);
 size = fifo_flush(&s->recv);
 SERIAL_EXIT_CRITICAL();
 return size;
}

int serial_open(int handle,uint8_t port,uint32_t bauds,uint8_t data_bit,uint8_t stop_bit)
{
 int status;
 serial_t *s;
 
 ASSERT_HANDLE(handle);
 s=(serial_t *)handle;	   

 if(s->registered == false){
 return -1;
 }
 
 status=s->driver->init(port,bauds,data_bit,stop_bit);
 if(status == -1){
  return -1;
 }

 s->port = port;
 s->complete=true;
 s->full = false;
 s->rxne_int_enable=true;
 s->driver->enable_rxne_int();

 return 0;
}


int serial_close(int handle)
{
 int status = 0;
 serial_t *s;
 
 ASSERT_HANDLE(handle);
 s=(serial_t *)handle;
 if(s->registered == false){
  return -1;
 }
 status=s->driver->deinit(s->port);
 s->port = -1;
 s->rxne_int_enable=false;
 s->driver->disable_rxne_int();
 s->txe_int_enable=false;
 s->driver->disable_txe_int();
 
 return status;
}

/*阻塞模式等待接收*/
int serial_select(int handle,uint32_t timeout)
{
 int size;
 serial_t *s;
 
 s=(serial_t *)handle;	 
 if(s->port == -1){
  return -1;
 } 
 ASSERT_HANDLE(handle);
 while(1){
 SERIAL_ENTER_CRITICAL(); 
 size=fifo_used_size(&s->recv);
 SERIAL_EXIT_CRITICAL(); 
 if(size == 0 && timeout > 0){
  osDelay(1);
  timeout--;
 }else{
  break;
 }
 }
 
 return size;
}
/*阻塞模式等待发送*/
int serial_complete(int handle,uint32_t timeout)
{
 int size;
 serial_t *s;
 ASSERT_HANDLE(handle);
 s =(serial_t *)handle;	 
 if(s->port == -1){
  return -1;
 }  
 while(s->complete == false && timeout-- >0){
 osDelay(1);
 }
 SERIAL_ENTER_CRITICAL();  
 size=fifo_free_size(&s->send);
 SERIAL_EXIT_CRITICAL(); 
 return size;
}

int serial_register_hal_driver(int handle,serial_hal_driver_t *driver)
{
  serial_t *s;
  
  ASSERT_HANDLE(handle);
  
  s=(serial_t *)handle; 

  ASSERT_NULL_POINTER(driver);
  ASSERT_NULL_POINTER(driver->init);
  ASSERT_NULL_POINTER(driver->deinit);
  ASSERT_NULL_POINTER(driver->enable_txe_int);
  ASSERT_NULL_POINTER(driver->disable_txe_int);
  ASSERT_NULL_POINTER(driver->enable_rxne_int);
  ASSERT_NULL_POINTER(driver->disable_rxne_int);
  s->driver = driver;
  s->registered = true;
  return 0;
}



/*中断处理*/
int isr_serial_get_byte_to_send(int handle,uint8_t *byte)
{
  int size;
  serial_t *s;
  
  ASSERT_HANDLE(handle);
  s=(serial_t *)handle;	 
  SERIAL_ENTER_CRITICAL();
  size = fifo_get(&s->send,byte,1);
  if(size == 0){
  s->complete = true;
  s->txe_int_enable = false;
  s->driver->disable_txe_int();
  }
  SERIAL_EXIT_CRITICAL();
  return size;
}

int isr_serial_put_byte_from_recv(int handle,uint8_t byte)
{
 
 int size;
 serial_t *s;
 
 ASSERT_HANDLE(handle);
 s=(serial_t *)handle;	 
 SERIAL_ENTER_CRITICAL();
 size =fifo_put(&s->recv,&byte,1);
 if(size == 0){
 s->full = true;
 s->rxne_int_enable = false;
 s->driver->disable_rxne_int();
 }
 SERIAL_EXIT_CRITICAL();
 return size;
}

int serial_create(int *handle,uint16_t rx_size,uint16_t tx_size)
{ 
 uint8_t *prx_buffer=NULL,*ptx_buffer=NULL;
 serial_t *s=NULL;
 
 ASSERT_NULL_POINTER(handle);

 if(rx_size > 0){
 prx_buffer = SERIAL_MALLOC(rx_size);
 if(prx_buffer == NULL){
 goto err_handle;
 }
 }
 
 if(tx_size > 0){
 ptx_buffer = SERIAL_MALLOC(tx_size);
 if(prx_buffer == NULL){
  goto err_handle;
 }
 }

 s=SERIAL_MALLOC(sizeof(serial_t));
 if(s == NULL){
 goto err_handle;
 }

 s->recv.pbuffer = prx_buffer;
 s->send.pbuffer = ptx_buffer;
 
 s->recv.size =rx_size;
 s->send.size =tx_size;
 
 s->recv.read =0;
 s->recv.write =0;
 
 s->send.read =0;
 s->send.write =0;

 s->driver = NULL;
 s->port = -1;
 s->registered =false;
 s->rxne_int_enable = false;
 s->txe_int_enable = false;
 s->complete = true;
 s->full =false;
 s->handle = (int)s;
 *handle = s->handle;
return 0;

err_handle:
 if(prx_buffer){
 SERIAL_FREE(prx_buffer);
 }
 
 if(ptx_buffer){
 SERIAL_FREE(ptx_buffer);
 }
 
 if(s){
 SERIAL_FREE(s);
 }

 return -1;		
}

int serial_destroy(int handle)
{
 serial_t *s;
 
 ASSERT_HANDLE(handle);

 s=( serial_t *)handle;

 SERIAL_FREE(s->recv.pbuffer);
 SERIAL_FREE(s->send.pbuffer);
 SERIAL_FREE(s);

 return 0;
}


