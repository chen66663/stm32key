/*----------------------------------------------------------------------------
 * 电源管理业务线程实现（G4）
 * 说明：Power 按键翻转整机状态；系统异常进入 Power Off 安全状态；
 *       看门狗喂狗逻辑统一放在本线程循环（禁止分散多处喂狗）
 *       本项目固定使用 CMSIS-RTOS2 / RTX5，直接调用 osXxx() 原生 API
 *       仅调用 BSP / WatchDog 与 RTX5 原生接口
 *---------------------------------------------------------------------------*/

#include "app_power_task.h"
#include "bsp_iwdg.h"
#include "com_config.h"

/* ==================== 模块私有句柄与状态 ==================== */
static osMessageQueueId_t   s_powerQueue = NULL;                /* 电源消息队列句柄 */
static SysPower     s_systemPowerState = SYS_POWER_OFF; /* 整机电源状态 */

/* ==================== 模块私有函数声明 ==================== */
static void app_power_msgHandle(const AppMsg *msg);

/* ==================== 对外接口实现 ==================== */

/**
 * @brief 电源模块初始化（创建队列、初始化看门狗、初始化电源相关 GPIO）
 * @return 错误码
 */
err_t app_power_init(void)
{
    /* TODO: osMessageQueueNew 创建电源队列、初始化看门狗、初始化 LED/电源 GPIO */
    (void)s_powerQueue;
    (void)s_systemPowerState;
    return ERR_OK;
}

/**
 * @brief 获取电源消息队列句柄，供其他模块发送电源事件
 * @return 队列句柄
 */
osMessageQueueId_t app_power_get_queue(void)
{
    /* TODO: 返回 s_powerQueue */
    return s_powerQueue;
}

/**
 * @brief 电源管理线程入口
 * @param arg 线程参数（未使用）
 */
void app_power_taskEntry(void *arg)
{
    (void)arg;
    /* TODO:
     * 1. 上电恢复历史状态
     * 2. 循环 osMessageQueueGet 接收电源消息并处理
     * 3. 每次循环统一喂狗 bsp_iwdg_feed_dog()
     */
    for (;;)
    {
        /* TODO: osMessageQueueGet -> app_power_msgHandle -> bsp_iwdg_feed_dog */
    }
}

/* ==================== 模块私有函数实现 ==================== */

/**
 * @brief 电源消息处理（翻转整机状态 / 进入安全状态）
 * @param msg 收到的电源消息
 */
static void app_power_msgHandle(const AppMsg *msg)
{
    (void)msg;
    /* TODO: 按 eventType 处理上下电翻转、异常安全状态，并通知 OLED 刷新 */
}
