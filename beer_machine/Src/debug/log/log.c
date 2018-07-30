#include "log.h"
#define  LOG_MODULE_LEVEL    LOG_LEVEL_DEBUG
#define  LOG_MODULE_NAME     "[log]"



void log_init(void)
{
#if LOG_USE_RTT > 0
SEGGER_RTT_Init();
#endif

#if  LOG_USE_SERIAL > 0
void log_serial_init();
log_serial_init();
#endif

log_debug("log init done.\r\n");
}


#if  LOG_USE_SERIAL > 0

int fputc(int ch, FILE *f)
{
void log_serial_write_byte(uint8_t);
log_serial_write_byte(ch);

return ch;
}
#endif




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


