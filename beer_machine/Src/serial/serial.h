#ifndef  __SERIAL_H__
#define  __SERIAL_H__

#include "stdint.h"

#ifdef  __cplusplus
# define SERIAL_BEGIN  extern "C" {
# define SERIAL_END    }
#else
# define SERIAL_BEGIN
# define SERIAL_END
#endif



SERIAL_BEGIN

#define  SERIAL_CNT             2
#define  BUFFER_SIZE_MAX        512

typedef struct 
{
int (*init)(uint8_t port,uint32_t bauds,uint8_t data_bit,uint8_t stop_bit);
int (*deinit)(uint8_t port);
void (*txe_int_enable)(void);
void (*txe_int_disable)(void);
void (*rxne_int_enable)(void);
void (*rxne_int_disable)(void);
}serial_hal_driver_t;


int serial_read(int handle,uint8_t *buff,int len);
int serial_write(int handle,uint8_t const *buff,int len);
int serial_flush(int handle);
int serial_open(int handle,uint8_t port,uint32_t bauds,uint8_t data_bit,uint8_t stop_bit);
int serial_close(int handle);
int serial_select(int handle,uint32_t timeout);
int serial_complete(int handle,uint32_t timeout);

int serial_create(int *handle);
int serial_destroy(int handle);
int serial_register_hal_driver(int handle,serial_hal_driver_t *hal);

int isr_serial_send(int handle,uint8_t *byte);
int isr_serial_recv(int handle,uint8_t byte);




SERIAL_END




#endif

