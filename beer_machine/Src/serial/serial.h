#ifndef  __SERIAL_H__
#define  __SERIAL_H__

#include "stdint.h"
#include "stdbool.h"
#include "string.h"

#ifdef __IAR_SYSTEMS_ICC__
#include <intrinsics.h>
#endif

#ifdef  __cplusplus
# define SERIAL_BEGIN  extern "C" {
# define SERIAL_END    }
#else
# define SERIAL_BEGIN
# define SERIAL_END
#endif

SERIAL_BEGIN

#define   LOG_SERIAL_MAX_INTERRUPT_PRIORITY         (5 << (8-4))


typedef struct 
{
int (*init)(uint8_t port,uint32_t bauds,uint8_t data_bit,uint8_t stop_bit);
int (*deinit)(uint8_t port);
void (*enable_txe_int)(void);
void (*disable_txe_int)(void);
void (*enable_rxne_int)(void);
void (*disable_rxne_int)(void);
}serial_hal_driver_t;


int serial_read(int handle,uint8_t *buff,int len);
int serial_write(int handle,uint8_t const *buff,int len);
int serial_flush(int handle);
int serial_open(int handle,uint8_t port,uint32_t bauds,uint8_t data_bit,uint8_t stop_bit);
int serial_close(int handle);
int serial_select(int handle,uint32_t timeout);
int serial_complete(int handle,uint32_t timeout);

int serial_create(int *handle,uint16_t rx_size,uint16_t tx_size);
int serial_destroy(int handle);
int serial_register_hal_driver(int handle,serial_hal_driver_t *driver);

int isr_serial_get_byte_to_send(int handle,uint8_t *byte);
int isr_serial_put_byte_from_recv(int handle,uint8_t byte);



#define  SERIAL_MALLOC(x)         pvPortMalloc((x))
#define  SERIAL_FREE(x)           vPortFree((x))



/*
*  serial critical configuration for IAR EWARM
*/

#ifdef __ICCARM__
  #if (defined (__ARM6M__) && (__CORE__ == __ARM6M__))             
    #define SERIAL_ENTER_CRITICAL()                            \
    {                                                          \
    unsigned int pri_mask;                                     \
    pri_mask = __get_PRIMASK();                                \
    __set_PRIMASK(1);
    
   #define SERIAL_EXIT_CRITICAL()                              \
    __set_PRIMASK(pri_mask);                                   \
    }
  #elif ((defined (__ARM7EM__) && (__CORE__ == __ARM7EM__)) || (defined (__ARM7M__) && (__CORE__ == __ARM7M__)))
   #ifndef  LOG_SERIAL_MAX_INTERRUPT_PRIORITY
   #define  LOG_SERIAL_MAX_INTERRUPT_PRIORITY                 (0x50)
   #endif
   #define SERIAL_ENTER_CRITICAL()                             \
   {                                                           \
   unsigned int base_pri;                                      \
   base_pri = __get_BASEPRI();                                 \
   __set_BASEPRI(LOG_SERIAL_MAX_INTERRUPT_PRIORITY);   

   #define SERIAL_EXIT_CRITICAL()                              \
   __set_BASEPRI(base_pri);                                    \
  }
  #endif
#endif


  
#ifdef __CC_ARM
  #if (defined __TARGET_ARCH_6S_M)
    #define SERIAL_ENTER_CRITICAL()                                             
   {                                                            \
    unsigned int pri_mask;                                      \
    register unsigned char PRIMASK __asm( "primask");           \
    pri_mask = PRIMASK;                                         \
    PRIMASK = 1u;                                               \
    __schedule_barrier();

    #define SERIAL_EXIT_CRITICAL()                              \
    PRIMASK = pri_mask;                                         \
     __schedule_barrier();                                      \
    }
  #elif (defined(__TARGET_ARCH_7_M) || defined(__TARGET_ARCH_7E_M))
    #ifndef  LOG_SERIAL_MAX_INTERRUPT_PRIORITY
    #define  LOG_SERIAL_MAX_INTERRUPT_PRIORITY                (0x50)
    #endif
    #define SERIAL_ENTER_CRITICAL()                             \
    {                                                           \
     unsigned int base_pri;                                     \
     register unsigned char BASEPRI __asm( "basepri");          \
      base_pri = BASEPRI;                                       \
      BASEPRI = LOG_SERIAL_MAX_INTERRUPT_PRIORITY;              \
      __schedule_barrier();

    #define SERIAL_EXIT_CRITICAL()                              \
     BASEPRI = base_pri;                                        \
     __schedule_barrier();                                      \
    }
  #endif
#endif  
  
  
  



SERIAL_END



#endif

