#include "app_power_task.h"
#include "app_msg.h"
#include "app_types.h"
#include "com_config.h"
#include "main.h"

static SysPower s_powerState = SYS_POWER_OFF;

static const SysPower s_powerStateMatrix[SYS_POWER_COUNT][POWER_EVENT_COUNT] =
{
    { SYS_POWER_ON,  SYS_POWER_OFF, SYS_POWER_OFF, SYS_POWER_ON  },
    { SYS_POWER_ON,  SYS_POWER_OFF, SYS_POWER_OFF, SYS_POWER_OFF }
};

static PowerEvent app_power_key_to_event(KeyEvent keyEvent);
static PowerEvent app_power_msg_to_event(const AppMsg *msg);
static void app_power_msg_handle(const AppMsg *msg);
static void app_power_enter_state(SysPower powerState);
static void app_power_notify_cd(SysPower powerState);
static void app_power_notify_oled(SysPower powerState);

err_t app_power_init(void)
{
    s_powerState = SYS_POWER_OFF;
    return ERR_OK;
}

void app_power_taskEntry(void const *argument)
{
    AppMsg *msg;

    (void)argument;
    (void)app_power_init();
    app_power_enter_state(SYS_POWER_OFF);

    for (;;)
    {
        msg = app_msg_get(g_powerMsgQueue, osWaitForever);
        if (msg != NULL)
        {
            app_power_msg_handle(msg);
            app_msg_free(msg);
        }
    }
}

static PowerEvent app_power_key_to_event(KeyEvent keyEvent)
{
    switch (keyEvent)
    {
    case EV_KEY_WK_UP_SHORT:
        return POWER_EVENT_KEY_ON;

    case EV_KEY_WK_UP_LONG:
        return POWER_EVENT_KEY_OFF;

    default:
        return POWER_EVENT_NONE;
    }
}

static PowerEvent app_power_msg_to_event(const AppMsg *msg)
{
    if (msg == NULL)
    {
        return POWER_EVENT_NONE;
    }

    if (msg->msgId == MSG_ID_KEY_EVENT)
    {
        return app_power_key_to_event((KeyEvent)msg->value);
    }

    if (msg->msgId == MSG_ID_POWER_STATE)
    {
        return (PowerEvent)msg->value;
    }

    return POWER_EVENT_NONE;
}

static void app_power_msg_handle(const AppMsg *msg)
{
    PowerEvent event = app_power_msg_to_event(msg);

    if ((event >= POWER_EVENT_COUNT) || (s_powerState >= SYS_POWER_COUNT))
    {
        return;
    }

    app_power_enter_state(s_powerStateMatrix[s_powerState][event]);
}

static void app_power_enter_state(SysPower powerState)
{
    if (powerState >= SYS_POWER_COUNT)
    {
        return;
    }

    if (s_powerState == powerState)
    {
        return;
    }

    s_powerState = powerState;
    app_power_notify_cd(s_powerState);
    app_power_notify_oled(s_powerState);
}

static void app_power_notify_cd(SysPower powerState)
{
    AppMsg msg;

    msg.srcModule = MODULE_POWER;
    msg.dstModule = MODULE_CD;
    msg.msgId = MSG_ID_POWER_STATE;
    msg.opt0 = 0U;
    msg.opt1 = 0U;
    msg.opt2 = 0U;
    msg.value = (uint16_t)powerState;

    (void)app_msg_send(g_cdMsgQueue, &msg);
}

static void app_power_notify_oled(SysPower powerState)
{
    OledMail *mail;

    if (g_oledMailQueue == NULL)
    {
        return;
    }

    mail = (OledMail *)osMailAlloc(g_oledMailQueue, 0U);
    if (mail == NULL)
    {
        return;
    }

    mail->srcModule = MODULE_POWER;
    mail->dstModule = MODULE_OLED;
    mail->msgId = MSG_ID_POWER_STATE;
    mail->powerState = powerState;
    mail->cdState = SYS_STATE_POWER_OFF;
    mail->cdDisplay = (uint8_t)OLED_CD_DISPLAY_NORMAL;
    mail->music = CD_MUSIC_MIN;

    if (osMailPut(g_oledMailQueue, mail) != osOK)
    {
        (void)osMailFree(g_oledMailQueue, mail);
    }
}
