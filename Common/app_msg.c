#include "app_msg.h"

uint8_t app_msg_send(osMessageQId queue, const AppMsg *msg)
{
    AppMsg *copy;

    if ((queue == NULL) || (msg == NULL) || (g_appMsgPool == NULL))
    {
        return FALSE;
    }

    copy = (AppMsg *)osPoolAlloc(g_appMsgPool);
    if (copy == NULL)
    {
        return FALSE;
    }

    *copy = *msg;
    if (osMessagePut(queue, (uint32_t)copy, 0U) != osOK)
    {
        (void)osPoolFree(g_appMsgPool, copy);
        return FALSE;
    }

    return TRUE;
}

AppMsg *app_msg_get(osMessageQId queue, uint32_t timeout)
{
    osEvent evt;

    if (queue == NULL)
    {
        return NULL;
    }

    evt = osMessageGet(queue, timeout);
    if (evt.status != osEventMessage)
    {
        return NULL;
    }

    return (AppMsg *)evt.value.p;
}

void app_msg_free(AppMsg *msg)
{
    if ((msg != NULL) && (g_appMsgPool != NULL))
    {
        (void)osPoolFree(g_appMsgPool, msg);
    }
}
