/*----------------------------------------------------------------------------
 * 按键扫描业务线程实现（G2）
 * 说明：本文件仅提供函数骨架，消抖/长短按状态机由 G2 实现
 *       仅可调用 BSP API 与 CMSIS-RTOS2（osXxx）原生接口
 *---------------------------------------------------------------------------*/

#include "app_key_task.h"
#include "com_config.h"
#include "cmsis_os2.h"
#include "bsp_gpio.h"

/* ==================== 模块私有静态变量 ==================== */
/* TODO(G2): 各按键消抖计数、按下时长计时、上一帧电平等私有状态 */
static uint8_t s_keyScanState;

/* ==================== 私有函数声明 ==================== */
static void app_key_scanHandle(void);

/* ==================== 接口实现 ==================== */
err_t app_key_init(void)
{
    /* TODO(G2): 按键 GPIO 初始化、私有状态复位 */
    s_keyScanState = 0;
    return ERR_OK;
}

void app_key_taskEntry(void *arg)
{
    (void)arg;

    /* TODO(G2): 周期扫描循环
     *   for (;;)
     *   {
     *       app_key_scanHandle();
     *       osDelay(KEY_SCAN_PERIOD_MS);
     *   }
     */
    for (;;)
    {
        app_key_scanHandle();
        osDelay(KEY_SCAN_PERIOD_MS);
    }
}

/* ==================== 私有函数实现 ==================== */
static void app_key_scanHandle(void)
{
    /* TODO(G2): 单次扫描 + 30ms 消抖 + 1.7s 长短按/释放判定
     *   按设计文档 6.1 映射为 MatrixEvent：
     *     WK_UP 短按 -> MATRIX_EVENT_POWER_ON   （目标 MODULE_POWER）
     *     WK_UP 长按 -> MATRIX_EVENT_POWER_OFF  （目标 MODULE_POWER）
     *     KEY_0 短按 -> MATRIX_EVENT_LOAD_EJECT  （目标 MODULE_CD）
     *     KEY_0 长按 -> MATRIX_EVENT_PREVIOUS    （目标 MODULE_CD）
     *     KEY_1 短按 -> MATRIX_EVENT_PLAY_PAUSE  （目标 MODULE_CD）
     *     KEY_1 长按 -> MATRIX_EVENT_NEXT        （目标 MODULE_CD）
     *     KEY_0/1 释放 -> MATRIX_EVENT_KEY_RELEASE（目标 MODULE_CD）
     *   组装 AppMsg（srcModule=MODULE_KEY, value=事件）后 osMessageQueuePut
     *   到 g_powerMsgQueue 或 g_cdMsgQueue。 */
}
