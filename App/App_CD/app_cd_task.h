#ifndef APP_CD_TASK_H
#define APP_CD_TASK_H

#include "cmsis_os.h"
#include "com_err.h"

err_t app_cd_init(void);
void app_cd_taskEntry(void const *argument);

#endif /* APP_CD_TASK_H */
