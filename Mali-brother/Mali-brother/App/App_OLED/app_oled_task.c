/*----------------------------------------------------------------------------
 * OLED 显示业务线程实现（G1）
 * 说明：接收 OLED 消息队列，维护三行固定显示内容
 *       第一行电源状态、第二行 CD 设备状态、第三行歌曲编号 Music 001~100
 *       仅调用 bsp_oled_iic 板级驱动与 CMSIS-RTOS2 / RTX5 原生 osXxx() 接口
 *---------------------------------------------------------------------------*/

#include "app_oled_task.h"
#include "bsp_oled_iic.h"
#include "com_config.h"

/* ==================== 模块私有句柄与状态 ==================== */
static osMessageQueueId_t s_oledQueue = NULL;   /* OLED 消息队列句柄 */

/* ==================== 模块私有函数声明 ==================== */
static void app_oled_msgHandle(const AppMsg *msg);

/* ==================== 对外接口实现 ==================== */

/**
 * @brief OLED 模块初始化（创建队列、初始化显示驱动）
 * @return 错误码
 */
err_t app_oled_init(void)
{
    /* TODO: osMessageQueueNew 创建 OLED 消息队列、初始化 OLED 驱动、绘制初始界面 */
    (void)s_oledQueue;
    return ERR_OK;
}

/**
 * @brief 获取 OLED 消息队列句柄，供其他模块发送刷新消息
 * @return 队列句柄
 */
osMessageQueueId_t app_oled_get_queue(void)
{
    /* TODO: 返回 s_oledQueue */
    return s_oledQueue;
}

/**
 * @brief OLED 显示线程入口
 * @param arg 线程参数（未使用）
 */
void app_oled_taskEntry(void *arg)
{
    (void)arg;
    /* TODO: 循环接收 OLED 消息队列，调用 app_oled_msgHandle 处理刷新 */
    for (;;)
    {
        /* TODO: osMessageQueueGet -> app_oled_msgHandle */
    }
}

/* ==================== 模块私有函数实现 ==================== */

/**
 * @brief OLED 消息处理（根据消息类型刷新对应显示行）
 * @param msg 收到的统一消息 AppMsg
 */
static void app_oled_msgHandle(const AppMsg *msg)
{
    (void)msg;
    /* TODO: 按 msgId 分别刷新：
     *   MSG_ID_POWER_STATE -> 第一行 Power 状态
     *   MSG_ID_CD_STATE    -> 第二行 CD 状态（opt0）+ 第三行 Music ###（value）
     */
}
