#include "app_cd_task.h"
#include "app_cd_fsm.h"
#include "app_msg.h"
#include "app_types.h"
#include "com_config.h"
#include "main.h"

static MatrixEvent app_cd_key_to_event(KeyEvent keyEvent);
static MatrixEvent app_cd_msg_to_event(const AppMsg *msg);
static void app_cd_msg_handle(const AppMsg *msg);
static void app_cd_repeat_release_handle(const AppMsg *msg);
static void app_cd_report_state(void);
static void app_cd_disc_timer_cb(void const *argument);
static void app_cd_repeat_timer_cb(void const *argument);
static void app_cd_send_self_event(MatrixEvent event);
static void app_cd_timer_refresh(MatrixEvent event);

static osTimerId s_discTimer = NULL;
static osTimerId s_repeatTimer = NULL;

osTimerDef(app_cd_disc_timer, app_cd_disc_timer_cb);
osTimerDef(app_cd_repeat_timer, app_cd_repeat_timer_cb);

err_t app_cd_init(void)
{
    if (app_cd_fsm_init() != ERR_OK)
    {
        return ERR_HW;
    }

    s_discTimer = osTimerCreate(osTimer(app_cd_disc_timer), osTimerOnce, NULL);
    s_repeatTimer = osTimerCreate(osTimer(app_cd_repeat_timer), osTimerPeriodic, NULL);
    if ((s_discTimer == NULL) || (s_repeatTimer == NULL))
    {
        return ERR_HW;
    }

    return ERR_OK;
}

void app_cd_taskEntry(void const *argument)
{
    AppMsg *msg;

    (void)argument;
    (void)app_cd_init();
    app_cd_report_state();

    for (;;)
    {
        msg = app_msg_get(g_cdMsgQueue, osWaitForever);
        if (msg != NULL)
        {
            app_cd_msg_handle(msg);
            app_msg_free(msg);
        }
    }
}

static MatrixEvent app_cd_key_to_event(KeyEvent keyEvent)
{
    switch (keyEvent)
    {
    case EV_KEY_0_SHORT:
        return MATRIX_EVENT_LOAD_EJECT;

    case EV_KEY_0_LONG:
        return MATRIX_EVENT_PREVIOUS;

    case EV_KEY_1_SHORT:
        return MATRIX_EVENT_PLAY_PAUSE;

    case EV_KEY_1_LONG:
        return MATRIX_EVENT_NEXT;

    case EV_KEY_0_OFF:
    case EV_KEY_1_OFF:
        return MATRIX_EVENT_KEY_RELEASE;

    default:
        return MATRIX_EVENT_NONE;
    }
}

static MatrixEvent app_cd_msg_to_event(const AppMsg *msg)
{
    if (msg == NULL)
    {
        return MATRIX_EVENT_NONE;
    }

    if (msg->msgId == MSG_ID_POWER_STATE)
    {
        return (msg->value == SYS_POWER_ON) ? MATRIX_EVENT_POWER_ON : MATRIX_EVENT_POWER_OFF;
    }

    if (msg->msgId == MSG_ID_KEY_EVENT)
    {
        return app_cd_key_to_event((KeyEvent)msg->value);
    }

    if (msg->msgId == MSG_ID_CD_EVENT)
    {
        if (msg->value == MATRIX_EVENT_TIMER_DISC_DONE)
        {
            return MATRIX_EVENT_DISC_ACTION_DONE;
        }
        if (msg->value == MATRIX_EVENT_TIMER_REPEAT)
        {
            return app_cd_fsm_get_repeat_event();
        }
        return (MatrixEvent)msg->value;
    }

    return MATRIX_EVENT_NONE;
}

static void app_cd_msg_handle(const AppMsg *msg)
{
    MatrixEvent event = app_cd_msg_to_event(msg);

    if (event == MATRIX_EVENT_NONE)
    {
        return;
    }

    app_cd_fsm_dispatch(event);

    if ((event == MATRIX_EVENT_PREVIOUS) || (event == MATRIX_EVENT_NEXT))
    {
        SysState state = app_cd_fsm_get_state();

        if (state == SYS_STATE_PAUSE)
        {
            app_cd_fsm_set_repeat_event(event);
        }
        else
        {
            app_cd_fsm_set_repeat_event(MATRIX_EVENT_NONE);
        }
    }
    else if (event == MATRIX_EVENT_KEY_RELEASE)
    {
        app_cd_repeat_release_handle(msg);
    }

    app_cd_timer_refresh(event);

    app_cd_report_state();
}

static void app_cd_repeat_release_handle(const AppMsg *msg)
{
    MatrixEvent repeatEvent;

    if ((msg == NULL) || (msg->msgId != MSG_ID_KEY_EVENT))
    {
        app_cd_fsm_set_repeat_event(MATRIX_EVENT_NONE);
        return;
    }

    repeatEvent = app_cd_fsm_get_repeat_event();
    if (((msg->value == EV_KEY_0_OFF) && (repeatEvent == MATRIX_EVENT_PREVIOUS)) ||
        ((msg->value == EV_KEY_1_OFF) && (repeatEvent == MATRIX_EVENT_NEXT)))
    {
        app_cd_fsm_set_repeat_event(MATRIX_EVENT_NONE);
    }
}

static void app_cd_timer_refresh(MatrixEvent event)
{
    MatrixEvent repeatEvent;
    SysState state = app_cd_fsm_get_state();

    if (s_discTimer != NULL)
    {
        if (app_cd_fsm_get_disc_done_event() == MATRIX_EVENT_DISC_ACTION_DONE)
        {
            (void)osTimerStart(s_discTimer, CD_DISC_ACTION_MS);
        }
        else if ((event == MATRIX_EVENT_DISC_ACTION_DONE) ||
                 (event == MATRIX_EVENT_POWER_OFF) ||
                 (state == SYS_STATE_POWER_OFF))
        {
            (void)osTimerStop(s_discTimer);
        }
    }

    repeatEvent = app_cd_fsm_get_repeat_event();
    if (s_repeatTimer != NULL)
    {
        if ((repeatEvent == MATRIX_EVENT_PREVIOUS) || (repeatEvent == MATRIX_EVENT_NEXT))
        {
            (void)osTimerStart(s_repeatTimer, CD_REPEAT_INTERVAL_MS);
        }
        else
        {
            (void)osTimerStop(s_repeatTimer);
        }
    }
}

static void app_cd_send_self_event(MatrixEvent event)
{
    AppMsg msg;

    msg.srcModule = MODULE_CD;
    msg.dstModule = MODULE_CD;
    msg.msgId = MSG_ID_CD_EVENT;
    msg.opt0 = 0U;
    msg.opt1 = 0U;
    msg.opt2 = 0U;
    msg.value = (uint16_t)event;

    (void)app_msg_send(g_cdMsgQueue, &msg);
}

static void app_cd_disc_timer_cb(void const *argument)
{
    (void)argument;
    app_cd_send_self_event(MATRIX_EVENT_TIMER_DISC_DONE);
}

static void app_cd_repeat_timer_cb(void const *argument)
{
    (void)argument;
    app_cd_send_self_event(MATRIX_EVENT_TIMER_REPEAT);
}

static void app_cd_report_state(void)
{
    static uint8_t s_reportReady;
    static SysState s_lastReportState;
    static uint16_t s_lastReportMusic;
    static uint8_t s_lastReportDisplay;
    OledMail *mail;
    SysState state;
    uint16_t music;
    MatrixEvent repeatEvent;
    uint8_t cdDisplay;

    if (g_cdOledMailQueue == NULL)
    {
        return;
    }

    state = app_cd_fsm_get_state();
    music = app_cd_fsm_get_music();
    repeatEvent = app_cd_fsm_get_repeat_event();
    if (repeatEvent == MATRIX_EVENT_PREVIOUS)
    {
        cdDisplay = (uint8_t)OLED_CD_DISPLAY_FAST_PREVIOUS;
    }
    else if (repeatEvent == MATRIX_EVENT_NEXT)
    {
        cdDisplay = (uint8_t)OLED_CD_DISPLAY_FAST_NEXT;
    }
    else
    {
        cdDisplay = (uint8_t)OLED_CD_DISPLAY_NORMAL;
    }

    if ((s_reportReady != 0U) &&
        (s_lastReportState == state) &&
        (s_lastReportMusic == music) &&
        (s_lastReportDisplay == cdDisplay))
    {
        return;
    }

    mail = (OledMail *)osMailAlloc(g_cdOledMailQueue, 0U);
    if (mail == NULL)
    {
        return;
    }

    mail->srcModule = MODULE_CD;
    mail->dstModule = MODULE_OLED;
    mail->msgId = MSG_ID_CD_STATE;
    mail->powerState = SYS_POWER_ON;
    mail->cdState = state;
    mail->cdDisplay = cdDisplay;
    mail->music = music;

    if (osMailPut(g_cdOledMailQueue, mail) != osOK)
    {
        (void)osMailFree(g_cdOledMailQueue, mail);
        return;
    }

    s_reportReady = 1U;
    s_lastReportState = state;
    s_lastReportMusic = music;
    s_lastReportDisplay = cdDisplay;
}
