#ifndef APP_CD_TASK_H
#define APP_CD_TASK_H

/*----------------------------------------------------------------------------
 * CD 播放器状态机业务线程（G3）
 * 说明：维护 CD 状态机，处理切碟/切歌逻辑，上电恢复历史播放状态
 *---------------------------------------------------------------------------*/

#include "com_type.h"
#include "com_err.h"
#include "app_types.h"

/* 任务入口 */
void app_cd_taskEntry(void *arg);

/* 任务初始化（创建队列/资源等，由 main 调用） */
err_t app_cd_init(void);

#endif /* APP_CD_TASK_H */
