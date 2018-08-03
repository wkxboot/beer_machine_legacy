#include "log.h"
#define  LOG_MODULE_LEVEL    LOG_LEVEL_DEBUG
#define  LOG_MODULE_NAME     "[log]"



void log_init(void)
{
#if LOG_USE_RTT > 0
SEGGER_RTT_Init();
#endif

#if  LOG_USE_SERIAL > 0
log_serial_init();
#endif

log_debug("log init done.\r\n");
}

uint16_t log_read(uint8_t *ptr_buffer,uint16_t buffer_size)
{
uint16_t read_cnt;
#if    LOG_USE_RTT > 0
read_cnt = SEGGER_RTT_Read(0,ptr_buffer,buffer_size);
#elif  LOG_USE_SERIAL > 0
read_cnt = log_serial_read(ptr_buffer,buffer_size);
#endif
return read_cnt;
}


__weak uint32_t log_time(void)
{
return 0;
}

__weak void log_assert_handler(int line,char *file_name)
{
   log_error("#############系统断言错误! ##############\r\n");
   log_error("断言文件：%s.\r\n",file_name);
   log_error("断言行号：%u.\r\n",line);
   while(1);
}


