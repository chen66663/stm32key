#include "app_cd_fsm.h"
#include "com_config.h"

static SysState s_curState;
static SysState s_lastState;
static uint16_t s_curMusic;
static MatrixEvent s_discDoneEvent;
static MatrixEvent s_repeatEvent;

static void app_cd_action_power_on(void);
static void app_cd_action_power_off(void);
static void app_cd_action_load(void);
static void app_cd_action_load_done(void);
static void app_cd_action_eject(void);
static void app_cd_action_eject_done(void);
static void app_cd_action_play(void);
static void app_cd_action_pause(void);
static void app_cd_action_previous(void);
static void app_cd_action_next(void);
static void app_cd_action_key_release(void);

static const SysState g_stateMatrix[SYS_STATE_COUNT][MATRIX_EVENT_COUNT] =
{
    {
        SYS_STATE_STOP, SYS_STATE_POWER_OFF, SYS_STATE_POWER_OFF, SYS_STATE_POWER_OFF,
        SYS_STATE_POWER_OFF, SYS_STATE_POWER_OFF, SYS_STATE_POWER_OFF, SYS_STATE_POWER_OFF
    },
    {
        SYS_STATE_NO_DISC, SYS_STATE_POWER_OFF, SYS_STATE_LOADING, SYS_STATE_NO_DISC,
        SYS_STATE_NO_DISC, SYS_STATE_NO_DISC, SYS_STATE_NO_DISC, SYS_STATE_NO_DISC
    },
    {
        SYS_STATE_LOADING, SYS_STATE_POWER_OFF, SYS_STATE_LOADING, SYS_STATE_STOP,
        SYS_STATE_LOADING, SYS_STATE_LOADING, SYS_STATE_LOADING, SYS_STATE_LOADING
    },
    {
        SYS_STATE_EJECTING, SYS_STATE_POWER_OFF, SYS_STATE_EJECTING, SYS_STATE_NO_DISC,
        SYS_STATE_EJECTING, SYS_STATE_EJECTING, SYS_STATE_EJECTING, SYS_STATE_EJECTING
    },
    {
        SYS_STATE_STOP, SYS_STATE_POWER_OFF, SYS_STATE_EJECTING, SYS_STATE_STOP,
        SYS_STATE_PLAY, SYS_STATE_STOP, SYS_STATE_STOP, SYS_STATE_STOP
    },
    {
        SYS_STATE_PLAY, SYS_STATE_POWER_OFF, SYS_STATE_EJECTING, SYS_STATE_PLAY,
        SYS_STATE_PAUSE, SYS_STATE_PLAY, SYS_STATE_PLAY, SYS_STATE_PLAY
    },
    {
        SYS_STATE_PAUSE, SYS_STATE_POWER_OFF, SYS_STATE_EJECTING, SYS_STATE_PAUSE,
        SYS_STATE_PLAY, SYS_STATE_PAUSE, SYS_STATE_PAUSE, SYS_STATE_PAUSE
    }
};

static const CdStateAction s_actionMatrix[SYS_STATE_COUNT][MATRIX_EVENT_COUNT] =
{
    {
        app_cd_action_power_on, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL
    },
    {
        NULL, app_cd_action_power_off, app_cd_action_load, NULL,
        NULL, NULL, NULL, NULL
    },
    {
        NULL, app_cd_action_power_off, NULL, app_cd_action_load_done,
        NULL, NULL, NULL, NULL
    },
    {
        NULL, app_cd_action_power_off, NULL, app_cd_action_eject_done,
        NULL, NULL, NULL, NULL
    },
    {
        NULL, app_cd_action_power_off, app_cd_action_eject, NULL,
        app_cd_action_play, NULL, NULL, NULL
    },
    {
        NULL, app_cd_action_power_off, app_cd_action_eject, NULL,
        app_cd_action_pause, app_cd_action_previous, app_cd_action_next, NULL
    },
    {
        NULL, app_cd_action_power_off, app_cd_action_eject, NULL,
        app_cd_action_play, app_cd_action_previous, app_cd_action_next, app_cd_action_key_release
    }
};

err_t app_cd_fsm_init(void)
{
    s_curState = SYS_STATE_POWER_OFF;
    s_lastState = SYS_STATE_STOP;
    s_curMusic = CD_MUSIC_MIN;
    s_discDoneEvent = MATRIX_EVENT_NONE;
    s_repeatEvent = MATRIX_EVENT_NONE;

    return ERR_OK;
}

void app_cd_fsm_dispatch(MatrixEvent event)
{
    SysState oldState;
    SysState targetState;
    CdStateAction action;

    if ((event >= MATRIX_EVENT_COUNT) || (s_curState >= SYS_STATE_COUNT))
    {
        return;
    }

    oldState = s_curState;
    action = s_actionMatrix[oldState][event];
    if (action != NULL)
    {
        action();
    }

    targetState = g_stateMatrix[oldState][event];
    if ((oldState == SYS_STATE_POWER_OFF) && (event == MATRIX_EVENT_POWER_ON))
    {
        targetState = s_curState;
    }
    s_curState = targetState;
}

void app_cd_fsm_set_repeat_event(MatrixEvent event)
{
    if ((event == MATRIX_EVENT_PREVIOUS) || (event == MATRIX_EVENT_NEXT))
    {
        s_repeatEvent = event;
    }
    else
    {
        s_repeatEvent = MATRIX_EVENT_NONE;
    }
}

MatrixEvent app_cd_fsm_get_repeat_event(void)
{
    return s_repeatEvent;
}

MatrixEvent app_cd_fsm_get_disc_done_event(void)
{
    return s_discDoneEvent;
}

SysState app_cd_fsm_get_state(void)
{
    return s_curState;
}

uint16_t app_cd_fsm_get_music(void)
{
    return s_curMusic;
}

static void app_cd_action_power_on(void)
{
    if ((s_lastState == SYS_STATE_POWER_OFF) ||
        (s_lastState == SYS_STATE_LOADING) ||
        (s_lastState == SYS_STATE_EJECTING))
    {
        s_lastState = SYS_STATE_STOP;
    }

    s_curState = s_lastState;
    s_discDoneEvent = MATRIX_EVENT_NONE;
    app_cd_fsm_set_repeat_event(MATRIX_EVENT_NONE);
}

static void app_cd_action_power_off(void)
{
    if ((s_curState != SYS_STATE_POWER_OFF) &&
        (s_curState != SYS_STATE_LOADING) &&
        (s_curState != SYS_STATE_EJECTING))
    {
        s_lastState = s_curState;
    }

    s_discDoneEvent = MATRIX_EVENT_NONE;
    app_cd_fsm_set_repeat_event(MATRIX_EVENT_NONE);
}

static void app_cd_action_load(void)
{
    s_discDoneEvent = MATRIX_EVENT_DISC_ACTION_DONE;
    app_cd_fsm_set_repeat_event(MATRIX_EVENT_NONE);
}

static void app_cd_action_load_done(void)
{
    s_discDoneEvent = MATRIX_EVENT_NONE;
    s_curMusic = CD_MUSIC_MIN;
}

static void app_cd_action_eject(void)
{
    s_discDoneEvent = MATRIX_EVENT_DISC_ACTION_DONE;
    app_cd_fsm_set_repeat_event(MATRIX_EVENT_NONE);
}

static void app_cd_action_eject_done(void)
{
    s_discDoneEvent = MATRIX_EVENT_NONE;
    s_curMusic = CD_MUSIC_MIN;
}

static void app_cd_action_play(void)
{
    app_cd_fsm_set_repeat_event(MATRIX_EVENT_NONE);
}

static void app_cd_action_pause(void)
{
    app_cd_fsm_set_repeat_event(MATRIX_EVENT_NONE);
}

static void app_cd_action_previous(void)
{
    if (s_curMusic <= CD_MUSIC_MIN)
    {
        s_curMusic = CD_MUSIC_MAX;
    }
    else
    {
        s_curMusic--;
    }
}

static void app_cd_action_next(void)
{
    if (s_curMusic >= CD_MUSIC_MAX)
    {
        s_curMusic = CD_MUSIC_MIN;
    }
    else
    {
        s_curMusic++;
    }
}

static void app_cd_action_key_release(void)
{
}
