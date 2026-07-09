#include "app_key_task.h"
#include "app_msg.h"
#include "app_types.h"
#include "bsp_key.h"
#include "com_config.h"

typedef struct
{
    uint8_t sampleCount;
    uint8_t confirmed;
    uint8_t longReported;
    uint32_t holdTimeMs;
} KeyTrack;

static KeyTrack s_keyTrack[BSP_KEY_COUNT];
static uint8_t s_suppressed[BSP_KEY_COUNT];
static osMessageQId s_cdQueue = NULL;
static osMessageQId s_powerQueue = NULL;
static osTimerId s_longTimer[BSP_KEY_COUNT];

static const AppMsg s_keyEventMsgTable[EV_KEY_COUNT] =
{
    { MODULE_KEY, MODULE_POWER, MSG_ID_KEY_EVENT, 0U, 0U, 0U, EV_KEY_WK_UP_SHORT },
    { MODULE_KEY, MODULE_POWER, MSG_ID_KEY_EVENT, 0U, 0U, 0U, EV_KEY_WK_UP_LONG  },
    { MODULE_KEY, MODULE_POWER, MSG_ID_KEY_EVENT, 0U, 0U, 0U, EV_KEY_WK_UP_OFF   },
    { MODULE_KEY, MODULE_CD,    MSG_ID_KEY_EVENT, 0U, 0U, 0U, EV_KEY_0_SHORT     },
    { MODULE_KEY, MODULE_CD,    MSG_ID_KEY_EVENT, 0U, 0U, 0U, EV_KEY_0_LONG      },
    { MODULE_KEY, MODULE_CD,    MSG_ID_KEY_EVENT, 0U, 0U, 0U, EV_KEY_0_OFF       },
    { MODULE_KEY, MODULE_CD,    MSG_ID_KEY_EVENT, 0U, 0U, 0U, EV_KEY_1_SHORT     },
    { MODULE_KEY, MODULE_CD,    MSG_ID_KEY_EVENT, 0U, 0U, 0U, EV_KEY_1_LONG      },
    { MODULE_KEY, MODULE_CD,    MSG_ID_KEY_EVENT, 0U, 0U, 0U, EV_KEY_1_OFF       }
};

static const uint16_t s_shortEvent[BSP_KEY_COUNT] =
{
    EV_KEY_WK_UP_SHORT,
    EV_KEY_0_SHORT,
    EV_KEY_1_SHORT
};

static const uint16_t s_longEvent[BSP_KEY_COUNT] =
{
    EV_KEY_WK_UP_LONG,
    EV_KEY_0_LONG,
    EV_KEY_1_LONG
};

static const uint16_t s_offEvent[BSP_KEY_COUNT] =
{
    EV_KEY_WK_UP_OFF,
    EV_KEY_0_OFF,
    EV_KEY_1_OFF
};

static void app_key_idle(void);
static void app_key_scan(void);
static void app_key_reset(uint8_t keyIndex);
static void app_key_suppress(uint8_t keyIndex);
static void app_key_report_release(uint8_t keyIndex, uint8_t longReported, uint32_t holdTimeMs);
static void app_key_send_event(uint16_t eventId);
static void app_key_long_timer_cb(void const *argument);

osTimerDef(app_key_wkup_long_timer, app_key_long_timer_cb);
osTimerDef(app_key_0_long_timer, app_key_long_timer_cb);
osTimerDef(app_key_1_long_timer, app_key_long_timer_cb);

void app_key_taskEntry(void const *argument)
{
    const osMessageQId *queues = (const osMessageQId *)argument;

    if (queues == NULL)
    {
        app_key_idle();
    }

    s_cdQueue = queues[0];
    s_powerQueue = queues[1];

    if ((s_cdQueue == NULL) || (s_powerQueue == NULL))
    {
        app_key_idle();
    }

    s_longTimer[BSP_KEY_WKUP] = osTimerCreate(osTimer(app_key_wkup_long_timer),
                                              osTimerOnce,
                                              (void *)KEY_TIMER_WKUP);
    s_longTimer[BSP_KEY_0] = osTimerCreate(osTimer(app_key_0_long_timer),
                                           osTimerOnce,
                                           (void *)KEY_TIMER_KEY0);
    s_longTimer[BSP_KEY_1] = osTimerCreate(osTimer(app_key_1_long_timer),
                                           osTimerOnce,
                                           (void *)KEY_TIMER_KEY1);

    if ((s_longTimer[BSP_KEY_WKUP] == NULL) ||
        (s_longTimer[BSP_KEY_0] == NULL) ||
        (s_longTimer[BSP_KEY_1] == NULL))
    {
        app_key_idle();
    }

    bsp_key_init();

    for (;;)
    {
        app_key_scan();
        osDelay(KEY_SCAN_INTERVAL_MS);
    }
}

static void app_key_idle(void)
{
    for (;;)
    {
        osDelay(1000U);
    }
}

static void app_key_reset(uint8_t keyIndex)
{
    if (keyIndex >= BSP_KEY_COUNT)
    {
        return;
    }

    s_keyTrack[keyIndex].sampleCount = 0U;
    s_keyTrack[keyIndex].confirmed = 0U;
    s_keyTrack[keyIndex].longReported = 0U;
    s_keyTrack[keyIndex].holdTimeMs = 0U;

    if (s_longTimer[keyIndex] != NULL)
    {
        (void)osTimerStop(s_longTimer[keyIndex]);
    }
}

static void app_key_suppress(uint8_t keyIndex)
{
    if (keyIndex < BSP_KEY_COUNT)
    {
        s_suppressed[keyIndex] = 1U;
        app_key_reset(keyIndex);
    }
}

static void app_key_scan(void)
{
    uint8_t rawState[BSP_KEY_COUNT];
    uint8_t wkupDown;
    uint8_t key0Down;
    uint8_t key1Down;
    uint8_t keyIndex;
    KeyTrack *track;

    for (keyIndex = 0U; keyIndex < BSP_KEY_COUNT; keyIndex++)
    {
        rawState[keyIndex] = bsp_key_read((BspKeyId)keyIndex);
        track = &s_keyTrack[keyIndex];

        if (rawState[keyIndex] == 0U)
        {
            s_suppressed[keyIndex] = 0U;
            if (track->confirmed == 0U)
            {
                track->sampleCount = 0U;
            }
            continue;
        }

        if ((s_suppressed[keyIndex] != 0U) || (track->confirmed != 0U))
        {
            if ((track->confirmed != 0U) && (rawState[keyIndex] != 0U))
            {
                track->holdTimeMs += KEY_SCAN_INTERVAL_MS;
            }
            continue;
        }

        if (track->sampleCount < KEY_PRESS_SAMPLES)
        {
            track->sampleCount++;
        }

        if (track->sampleCount >= KEY_PRESS_SAMPLES)
        {
            track->confirmed = 1U;
            track->longReported = 0U;
            track->holdTimeMs = 0U;
            if (s_longTimer[keyIndex] != NULL)
            {
                (void)osTimerStart(s_longTimer[keyIndex], KEY_LONG_PRESS_MS);
            }
        }
    }

    wkupDown = ((rawState[BSP_KEY_WKUP] != 0U) &&
                (s_keyTrack[BSP_KEY_WKUP].confirmed != 0U)) ? 1U : 0U;
    key0Down = ((rawState[BSP_KEY_0] != 0U) &&
                ((s_keyTrack[BSP_KEY_0].confirmed != 0U) ||
                 (s_suppressed[BSP_KEY_0] != 0U))) ? 1U : 0U;
    key1Down = ((rawState[BSP_KEY_1] != 0U) &&
                ((s_keyTrack[BSP_KEY_1].confirmed != 0U) ||
                 (s_suppressed[BSP_KEY_1] != 0U))) ? 1U : 0U;

    if (wkupDown != 0U)
    {
        if (key0Down != 0U)
        {
            app_key_suppress(BSP_KEY_0);
        }
        if (key1Down != 0U)
        {
            app_key_suppress(BSP_KEY_1);
        }
    }
    else if ((key0Down != 0U) && (key1Down != 0U))
    {
        app_key_suppress(BSP_KEY_0);
        app_key_suppress(BSP_KEY_1);
    }

    for (keyIndex = 0U; keyIndex < BSP_KEY_COUNT; keyIndex++)
    {
        track = &s_keyTrack[keyIndex];

        if ((s_suppressed[keyIndex] == 0U) &&
            (track->confirmed != 0U) &&
            (rawState[keyIndex] == 0U))
        {
            uint8_t longReported = track->longReported;
            uint32_t holdTimeMs = track->holdTimeMs;

            app_key_reset(keyIndex);
            app_key_report_release(keyIndex, longReported, holdTimeMs);
        }
    }
}

static void app_key_report_release(uint8_t keyIndex, uint8_t longReported, uint32_t holdTimeMs)
{
    if (keyIndex >= BSP_KEY_COUNT)
    {
        return;
    }

    if (longReported == 0U)
    {
        if (holdTimeMs >= KEY_LONG_PRESS_MS)
        {
            app_key_send_event(s_longEvent[keyIndex]);
            longReported = 1U;
        }
        else if ((holdTimeMs + KEY_LONG_MARGIN_MS) < KEY_LONG_PRESS_MS)
        {
            app_key_send_event(s_shortEvent[keyIndex]);
        }
    }

    app_key_send_event(s_offEvent[keyIndex]);
}

static void app_key_long_timer_cb(void const *argument)
{
    uint32_t timerId = (uint32_t)argument;

    if (timerId >= KEY_TIMER_COUNT)
    {
        return;
    }

    if ((s_keyTrack[timerId].confirmed != 0U) &&
        (s_keyTrack[timerId].longReported == 0U) &&
        (s_suppressed[timerId] == 0U) &&
        (bsp_key_read((BspKeyId)timerId) != 0U))
    {
        s_keyTrack[timerId].longReported = 1U;
        app_key_send_event(s_longEvent[timerId]);
    }
}

static void app_key_send_event(uint16_t eventId)
{
    osMessageQId targetQueue;

    if (eventId >= EV_KEY_COUNT)
    {
        return;
    }

    targetQueue = (s_keyEventMsgTable[eventId].dstModule == MODULE_POWER) ? s_powerQueue : s_cdQueue;
    if (targetQueue != NULL)
    {
        (void)app_msg_send(targetQueue, &s_keyEventMsgTable[eventId]);
    }
}
