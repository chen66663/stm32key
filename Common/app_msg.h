#ifndef APP_MSG_H
#define APP_MSG_H

#include "cmsis_os.h"
#include "app_types.h"
#include "com_type.h"

extern osPoolId g_appMsgPool;

uint8_t app_msg_send(osMessageQId queue, const AppMsg *msg);
AppMsg *app_msg_get(osMessageQId queue, uint32_t timeout);
void app_msg_free(AppMsg *msg);

#endif /* APP_MSG_H */
