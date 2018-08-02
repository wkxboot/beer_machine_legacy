#include "cmsis_os.h"
#include "tasks_init.h"
#include "log.h"
#define  LOG_MODULE_LEVEL    LOG_LEVEL_DEBUG
#define  LOG_MODULE_NAME     "[tasks]"

EventGroupHandle_t tasks_sync_evt_group_hdl;


void tasks_init()
{
 tasks_sync_evt_group_hdl=xEventGroupCreate(); 
 log_assert(tasks_sync_evt_group_hdl);
}

