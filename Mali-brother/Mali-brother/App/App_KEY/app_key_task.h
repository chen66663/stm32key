#ifndef APP_KEY_TASK_H
#define APP_KEY_TASK_H

/*----------------------------------------------------------------------------
 * 按键扫描业务线程（G2）
 * 说明：负责按键扫描、30ms 消抖、1.7s 长短按识别，产生按键消息
 *---------------------------------------------------------------------------*/

#include "com_type.h"
#include "com_err.h"
#include "app_types.h"

/* 任务入口 */
void app_key_taskEntry(void *arg);

/* 任务初始化（创建队列/资源等，由 main 调用） */
err_t app_key_init(void);

#endif /* APP_KEY_TASK_H */
