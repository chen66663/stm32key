#ifndef APP_POWER_TASK_H
#define APP_POWER_TASK_H

/*----------------------------------------------------------------------------
 * 电源管理业务线程对外接口（G4）
 * 说明：整机上下电逻辑、系统安全状态管理；看门狗喂狗统一放在本线程循环
 *---------------------------------------------------------------------------*/

#include "com_type.h"
#include "com_err.h"
#include "app_types.h"
#include "cmsis_os2.h"

/* 电源模块初始化（创建队列、初始化看门狗、初始化电源 GPIO） */
err_t app_power_init(void);

/* 获取电源消息队列句柄 */
osMessageQueueId_t app_power_get_queue(void);

/* 电源管理线程入口 */
void app_power_taskEntry(void *arg);

#endif /* APP_POWER_TASK_H */
