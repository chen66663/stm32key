#ifndef APP_OLED_TASK_H
#define APP_OLED_TASK_H

/*----------------------------------------------------------------------------
 * OLED 显示业务线程（G1）
 * 说明：OLED 显示线程入口与对外接口声明
 *       负责接收 OLED 消息队列，刷新三行固定显示内容
 *---------------------------------------------------------------------------*/

#include "com_type.h"
#include "com_err.h"
#include "app_types.h"
#include "cmsis_os2.h"

/* 跨模块全局队列句柄（OLED 消息入口） */
extern osMessageQueueId_t g_oledMsgQueue;

/* ==================== 对外接口 ==================== */

/* OLED 模块资源初始化（创建队列等），由系统初始化阶段调用 */
err_t app_oled_init(void);

/* OLED 显示业务线程入口 */
void app_oled_taskEntry(void *arg);

#endif /* APP_OLED_TASK_H */
