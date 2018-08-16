#ifndef  __LOG_SERIAL_H__
#define  __LOG_SERIAL_H__

#define  LOG_SERIAL_PRINTF_BUFFER_SIZE   128
#define  LOG_SERIAL_RX_BUFFER_SIZE       32
#define  LOG_SERIAL_TX_BUFFER_SIZE       512


#define  LOG_SERIAL_PORT                 1
#define  LOG_SERIAL_BAUDRATE             115200
#define  LOG_SERIAL_DATA_BITS            8
#define  LOG_SERIAL_STOP_BITS            1


int log_serial_init();
int log_serial_read(uint8_t *pbuffer,uint16_t size);

int log_serial_sprintf(char *out, const char *format, ...);
int log_serial_snprintf( char *buf, unsigned int count, const char *format, ... );
int log_serial_printf(const char *format, ...);


















#endif