/*----------------------------------------------------------------------------
 * 系统主入口
 * 说明：RTOS 初始化、全局消息队列创建、四大业务线程创建入口
 *       本文件仅做框架编排，不实现具体业务逻辑
 *       固定使用 CMSIS-RTOS2 / RTX5，直接调用 osXxx() 原生 API
 *---------------------------------------------------------------------------*/

#include "main.h"
#include "bsp_gpio.h"
#include "bsp_rcc.h"
#include "app_key_task.h"
#include "app_cd_task.h"
#include "app_oled_task.h"
#include "app_power_task.h"

/* ==================== 跨模块全局消息队列句柄定义 ==================== */
osMessageQueueId_t g_cdMsgQueue    = NULL;
osMessageQueueId_t g_oledMsgQueue  = NULL;
osMessageQueueId_t g_powerMsgQueue = NULL;

/* ----------------------------------------------------------------------------
 * 板级底层硬件初始化
 * -------------------------------------------------------------------------- */
err_t sys_bsp_init(void)
{
    /* TODO: 时钟、GPIO 等底层初始化 */
    return ERR_OK;
}

/* ----------------------------------------------------------------------------
 * 全局消息队列创建
 * -------------------------------------------------------------------------- */
err_t sys_queue_init(void)
{
    /* TODO: osMessageQueueNew() 创建 CD / OLED / Power 消息队列
     *   三条队列统一传递 AppMsg（设计文档 5.1 节统一消息载体）
     *   g_cdMsgQueue    = osMessageQueueNew(QUEUE_CD_LEN, sizeof(AppMsg), NULL);
     *   g_oledMsgQueue  = osMessageQueueNew(QUEUE_OLED_LEN, sizeof(AppMsg), NULL);
     *   g_powerMsgQueue = osMessageQueueNew(QUEUE_POWER_LEN, sizeof(AppMsg), NULL);
     */
    return ERR_OK;
}

/* ----------------------------------------------------------------------------
 * 四大业务线程创建
 * -------------------------------------------------------------------------- */
err_t sys_task_init(void)
{
    /* TODO: osThreadNew() 创建 KEY / Power / CD / OLED 四大线程
     *   栈大小取 TASK_XXX_STACK_SIZE，优先级取 TASK_XXX_PRIORITY
     */
    return ERR_OK;
}

/* ----------------------------------------------------------------------------
 * 主函数
 * -------------------------------------------------------------------------- */
int main(void)
{
    /* 1. 内核初始化 */
    osKernelInitialize();

    /* 2. 板级硬件初始化 */
    sys_bsp_init();

    /* 3. 全局消息队列创建 */
    sys_queue_init();

    /* 4. 四大业务线程创建 */
    sys_task_init();

    /* 5. 启动内核调度 */
    osKernelStart();

    for (;;)
    {
        /* 调度器启动后不应到达此处 */
    }
}
