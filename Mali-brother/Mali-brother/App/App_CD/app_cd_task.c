/*----------------------------------------------------------------------------
 * CD 播放器业务线程实现（G3）
 * 说明：本文件仅提供函数骨架，状态机矩阵/动作逻辑见 app_cd_fsm.c
 *       线程职责：创建 CD 队列、收 AppMsg、调 FSM 派发、回报状态给 OLED
 *       跨模块通信仅通过 CMSIS-RTOS2（osMessageQueue）传递 AppMsg
 *---------------------------------------------------------------------------*/

#include "app_cd_task.h"
#include "app_cd_fsm.h"
#include "com_config.h"
#include "cmsis_os2.h"
#include "app_types.h"

/* ==================== 模块私有函数声明 ==================== */
static void app_cd_msgHandle(const AppMsg *msg);
static void app_cd_report_state(void);

/* ==================== 接口实现 ==================== */
err_t app_cd_init(void)
{
    /* TODO(G3): 初始化状态机（队列由 main 的 sys_queue_init 统一创建） */
    return app_cd_fsm_init();
}

void app_cd_taskEntry(void *arg)
{
    (void)arg;

    /* TODO(G3): 收消息 -> 解析 MatrixEvent -> FSM 派发 -> 回报 OLED
     *   AppMsg msg;
     *   for (;;)
     *   {
     *       if (osMessageQueueGet(g_cdMsgQueue, &msg, NULL, osWaitForever) == osOK)
     *       {
     *           app_cd_msgHandle(&msg);
     *       }
     *       // 也可在此处理 Load/Eject 3s、PAUSE 下 0.5s/首 连续切歌的超时
     *   }
     */
    for (;;)
    {
        osDelay(CD_LONG_SWITCH_MS);
    }
}

/* ==================== 私有函数实现 ==================== */

/**
 * @brief CD 消息处理：从 AppMsg 取出事件，驱动状态机并回报状态
 * @param msg 收到的统一消息
 */
static void app_cd_msgHandle(const AppMsg *msg)
{
    /* TODO(G3):
     *   1. 从 msg->value/opt 取出 MatrixEvent
     *   2. app_cd_fsm_dispatch(event);
     *   3. app_cd_report_state();
     */
    (void)msg;
}

/**
 * @brief 向 OLED 线程上报当前 CD 状态与歌曲编号
 */
static void app_cd_report_state(void)
{
    /* TODO(G3): 组装 AppMsg（srcModule=MODULE_CD, dstModule=MODULE_OLED,
     *           msgId=MSG_ID_CD_STATE, value=曲目编号, opt0=状态），
     *           osMessageQueuePut(g_oledMsgQueue, &msg, 0, 0); */
}
