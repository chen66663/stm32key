#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_os.h"

#include "main.h"
#include "app_cd_task.h"
#include "app_key_task.h"
#include "app_msg.h"
#include "app_oled_task.h"
#include "app_power_task.h"
#include "app_types.h"
#include "bsp_iwdg.h"
#include "com_config.h"
#include "com_type.h"

osMessageQId g_cdMsgQueue = NULL;
osMessageQId g_oledMsgQueue = NULL;
osMessageQId g_powerMsgQueue = NULL;
osMailQId g_cdOledMailQueue = NULL;
osPoolId g_appMsgPool = NULL;

static osMessageQId s_keyQueues[2];

osMessageQDef(cdMsgQueue, QUEUE_CD_LEN, uint32_t);
osMessageQDef(oledMsgQueue, QUEUE_OLED_LEN, uint32_t);
osMessageQDef(powerMsgQueue, QUEUE_POWER_LEN, uint32_t);
osMailQDef(cdOledMailQueue, MAIL_CD_OLED_LEN, OledMail);
osPoolDef(appMsgPool, APP_MSG_POOL_LEN, AppMsg);
osThreadDef(app_key_taskEntry, osPriorityNormal, 1U, TASK_KEY_STACK_SIZE);
osThreadDef(app_power_taskEntry, osPriorityNormal, 1U, TASK_POWER_STACK_SIZE);
osThreadDef(app_cd_taskEntry, osPriorityNormal, 1U, TASK_CD_STACK_SIZE);
osThreadDef(app_oled_taskEntry, osPriorityNormal, 1U, TASK_OLED_STACK_SIZE);

static uint8_t sys_queue_init(void)
{
    g_appMsgPool = osPoolCreate(osPool(appMsgPool));
    g_cdMsgQueue = osMessageCreate(osMessageQ(cdMsgQueue), NULL);
    g_oledMsgQueue = osMessageCreate(osMessageQ(oledMsgQueue), NULL);
    g_powerMsgQueue = osMessageCreate(osMessageQ(powerMsgQueue), NULL);
    g_cdOledMailQueue = osMailCreate(osMailQ(cdOledMailQueue), NULL);

    return ((g_appMsgPool != NULL) &&
            (g_cdMsgQueue != NULL) &&
            (g_oledMsgQueue != NULL) &&
            (g_powerMsgQueue != NULL) &&
            (g_cdOledMailQueue != NULL)) ? TRUE : FALSE;
}

static uint8_t sys_task_init(void)
{
    s_keyQueues[0] = g_cdMsgQueue;
    s_keyQueues[1] = g_powerMsgQueue;

    if (osThreadCreate(osThread(app_key_taskEntry), (void *)s_keyQueues) == NULL)
    {
        return FALSE;
    }
    if (osThreadCreate(osThread(app_power_taskEntry), NULL) == NULL)
    {
        return FALSE;
    }
    if (osThreadCreate(osThread(app_cd_taskEntry), NULL) == NULL)
    {
        return FALSE;
    }
    if (osThreadCreate(osThread(app_oled_taskEntry), NULL) == NULL)
    {
        return FALSE;
    }

    return TRUE;
}

int main(void)
{
    SystemCoreClockUpdate();
    bsp_iwdg_init();

    if ((sys_queue_init() == FALSE) || (sys_task_init() == FALSE))
    {
        for (;;)
        {
            osDelay(1000U);
        }
    }

    osThreadTerminate(osThreadGetId());
    return 0;
}
