#ifndef MAIN_H
#define MAIN_H

/*----------------------------------------------------------------------------
 * 系统主入口头文件
 * 说明：声明系统初始化与全局共享句柄
 *       负责 RTOS 初始化、全局队列创建、四大业务线程创建入口
 *       本项目固定使用 CMSIS-RTOS2 / RTX5，业务直接调用 osXxx() 原生 API
 *---------------------------------------------------------------------------*/

#include "com_type.h"
#include "com_err.h"
#include "com_config.h"
#include "app_types.h"
#include "cmsis_os2.h"

/* ==================== 跨模块全局消息队列句柄 ==================== */
/* 说明：跨模块业务通信统一通过以下队列传递结构体消息 */
extern osMessageQueueId_t g_cdMsgQueue;       /* 发往 CD 线程的消息队列 */
extern osMessageQueueId_t g_oledMsgQueue;     /* 发往 OLED 线程的消息队列 */
extern osMessageQueueId_t g_powerMsgQueue;    /* 发往 Power 线程的消息队列 */

/* ==================== 系统初始化接口 ==================== */
err_t sys_bsp_init(void);               /* 板级底层硬件初始化 */
err_t sys_queue_init(void);             /* 全局消息队列创建 */
err_t sys_task_init(void);              /* 四大业务线程创建 */

#endif /* MAIN_H */
