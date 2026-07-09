#ifndef APP_POWER_TASK_H
#define APP_POWER_TASK_H

#include "cmsis_os.h"
#include "com_err.h"

err_t app_power_init(void);
void app_power_taskEntry(void const *argument);

#endif /* APP_POWER_TASK_H */
