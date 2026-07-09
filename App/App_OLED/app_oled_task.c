#include "app_oled_task.h"
#include "app_oled_anim.h"
#include "app_types.h"
#include "bsp_oled_font.h"
#include "bsp_oled_iic.h"
#include "com_config.h"
#include "main.h"

#define OLED_ASCII_FONT_WIDTH 6U

static OledState s_oledState = OLED_STATE_OFF;
static SysPower s_powerState = SYS_POWER_OFF;
static SysState s_cdState = SYS_STATE_POWER_OFF;
static uint8_t s_cdDisplay = (uint8_t)OLED_CD_DISPLAY_NORMAL;
static uint16_t s_music = CD_MUSIC_MIN;
static uint8_t s_showPowerOn = 0U;
static osTimerId s_powerOnTimer = NULL;

static const OledState s_oledStateMatrix[OLED_STATE_COUNT][OLED_EVENT_COUNT] =
{
    { OLED_STATE_OFF,   OLED_STATE_READY,  OLED_STATE_OFF,    OLED_STATE_OFF    },
    { OLED_STATE_OFF,   OLED_STATE_READY,  OLED_STATE_UPDATE, OLED_STATE_UPDATE },
    { OLED_STATE_OFF,   OLED_STATE_READY,  OLED_STATE_UPDATE, OLED_STATE_UPDATE },
    { OLED_STATE_ERROR, OLED_STATE_ERROR,  OLED_STATE_ERROR,  OLED_STATE_ERROR  }
};

static void app_oled_mail_handle(const OledMail *mail);
static void app_oled_dispatch(OledEvent event);
static void app_oled_render(void);
static void app_oled_power_on_timer_cb(void const *argument);
static void app_oled_send_refresh_event(void);
static void app_oled_show_centered_string(uint8_t page, const char *str);
static void app_oled_build_music_text(char *text, uint16_t music);
static uint8_t app_oled_string_len(const char *str);
static const char *app_oled_power_text(SysPower state);
static const char *app_oled_cd_text(SysState state);

osTimerDef(app_oled_power_on_timer, app_oled_power_on_timer_cb);

err_t app_oled_init(void)
{
    s_oledState = OLED_STATE_READY;
    s_powerState = SYS_POWER_OFF;
    s_cdState = SYS_STATE_POWER_OFF;
    s_cdDisplay = (uint8_t)OLED_CD_DISPLAY_NORMAL;
    s_music = CD_MUSIC_MIN;
    s_showPowerOn = 0U;

    if (bsp_oled_iic_init() != ERR_OK)
    {
        s_oledState = OLED_STATE_ERROR;
        return ERR_HW;
    }

    s_powerOnTimer = osTimerCreate(osTimer(app_oled_power_on_timer), osTimerOnce, NULL);
    if (s_powerOnTimer == NULL)
    {
        s_oledState = OLED_STATE_ERROR;
        return ERR_HW;
    }

    app_oled_render();
    return ERR_OK;
}

void app_oled_taskEntry(void const *argument)
{
    osEvent evt;
    OledMail *mail;

    (void)argument;
    (void)app_oled_init();

    for (;;)
    {
        if (g_oledMailQueue != NULL)
        {
            evt = osMailGet(g_oledMailQueue, OLED_TASK_PERIOD_MS);
            if (evt.status == osEventMail)
            {
                mail = (OledMail *)evt.value.p;
                app_oled_mail_handle(mail);
                (void)osMailFree(g_oledMailQueue, mail);
            }
        }
        else
        {
            osDelay(OLED_TASK_PERIOD_MS);
        }
    }
}

void app_oled_show_bitmap(uint8_t x,
                          uint8_t page,
                          uint8_t width,
                          uint8_t pageCount,
                          const uint8_t *bitmap)
{
    if (s_powerState == SYS_POWER_OFF)
    {
        return;
    }

    bsp_oled_iic_show_bitmap(x, page, width, pageCount, bitmap);
    bsp_oled_iic_refresh();
}

void app_oled_show_chinese16(uint8_t x, uint8_t page, const uint8_t *font16x16)
{
    if (s_powerState == SYS_POWER_OFF)
    {
        return;
    }

    bsp_oled_iic_show_chinese16(x, page, font16x16);
    bsp_oled_iic_refresh();
}

void app_oled_show_mali_brother_group(uint8_t x, uint8_t page)
{
    uint8_t index;
    uint8_t drawX = x;

    if (s_powerState == SYS_POWER_OFF)
    {
        return;
    }

    for (index = 0U; index < OLED_FONT_MALI_GROUP_COUNT; index++)
    {
        bsp_oled_iic_show_chinese16(drawX, page, g_oledFontMaliBrotherGroup[index]);
        drawX = (uint8_t)(drawX + OLED_FONT_CHINESE16_WIDTH);
    }

    bsp_oled_iic_refresh();
}

static void app_oled_mail_handle(const OledMail *mail)
{
    if (mail == NULL)
    {
        return;
    }

    if (mail->msgId == MSG_ID_POWER_STATE)
    {
        s_powerState = mail->powerState;
        if (s_powerState == SYS_POWER_OFF)
        {
            s_cdState = SYS_STATE_POWER_OFF;
            s_cdDisplay = (uint8_t)OLED_CD_DISPLAY_NORMAL;
            s_showPowerOn = 0U;
            if (s_powerOnTimer != NULL)
            {
                (void)osTimerStop(s_powerOnTimer);
            }
        }
        else
        {
            s_showPowerOn = 1U;
            (void)app_oled_play_boot_animation();
            if (s_powerOnTimer != NULL)
            {
                (void)osTimerStart(s_powerOnTimer, OLED_POWER_ON_DISPLAY_MS);
            }
        }
        app_oled_dispatch((s_powerState == SYS_POWER_ON) ? OLED_EVENT_POWER_ON : OLED_EVENT_POWER_OFF);
        app_oled_render();
    }
    else if (mail->msgId == MSG_ID_CD_STATE)
    {
        s_cdState = mail->cdState;
        s_cdDisplay = mail->cdDisplay;
        if ((mail->music >= CD_MUSIC_MIN) && (mail->music <= CD_MUSIC_MAX))
        {
            s_music = mail->music;
        }

        app_oled_dispatch(OLED_EVENT_CD_UPDATE);
        app_oled_render();
    }
    else if (mail->msgId == MSG_ID_OLED_REFRESH)
    {
        if (s_powerState == SYS_POWER_ON)
        {
            s_showPowerOn = 0U;
        }
        app_oled_dispatch(OLED_EVENT_REFRESH);
        app_oled_render();
    }
}

static void app_oled_dispatch(OledEvent event)
{
    if ((event >= OLED_EVENT_COUNT) || (s_oledState >= OLED_STATE_COUNT))
    {
        return;
    }

    s_oledState = s_oledStateMatrix[s_oledState][event];
}

static void app_oled_power_on_timer_cb(void const *argument)
{
    (void)argument;
    app_oled_send_refresh_event();
}

static void app_oled_send_refresh_event(void)
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

    mail->srcModule = MODULE_OLED;
    mail->dstModule = MODULE_OLED;
    mail->msgId = MSG_ID_OLED_REFRESH;
    mail->powerState = s_powerState;
    mail->cdState = s_cdState;
    mail->cdDisplay = s_cdDisplay;
    mail->music = s_music;

    if (osMailPut(g_oledMailQueue, mail) != osOK)
    {
        (void)osMailFree(g_oledMailQueue, mail);
    }
}

static void app_oled_render(void)
{
    char musicText[10];

    bsp_oled_iic_clear();
    if (s_powerState == SYS_POWER_OFF)
    {
        app_oled_show_centered_string(0U, app_oled_power_text(s_powerState));
        bsp_oled_iic_refresh();
        return;
    }

    app_oled_build_music_text(musicText, s_music);
    app_oled_show_centered_string(0U, app_oled_power_text(s_powerState));
    app_oled_show_centered_string(2U, app_oled_cd_text(s_cdState));
    app_oled_show_centered_string(4U, musicText);
    bsp_oled_iic_refresh();

    if (s_oledState == OLED_STATE_UPDATE)
    {
        s_oledState = OLED_STATE_READY;
    }
}

static void app_oled_show_centered_string(uint8_t page, const char *str)
{
    uint8_t len;
    uint16_t pixelWidth;
    uint8_t x;

    if (str == NULL)
    {
        return;
    }

    len = app_oled_string_len(str);
    pixelWidth = (uint16_t)len * OLED_ASCII_FONT_WIDTH;
    if (pixelWidth >= OLED_WIDTH)
    {
        x = 0U;
    }
    else
    {
        x = (uint8_t)((OLED_WIDTH - pixelWidth) / 2U);
    }

    bsp_oled_iic_show_string(x, page, str);
}

static void app_oled_build_music_text(char *text, uint16_t music)
{
    if (text == NULL)
    {
        return;
    }

    if (music > CD_MUSIC_MAX)
    {
        music = CD_MUSIC_MAX;
    }
    if (music < CD_MUSIC_MIN)
    {
        music = CD_MUSIC_MIN;
    }

    text[0] = 'M';
    text[1] = 'u';
    text[2] = 's';
    text[3] = 'i';
    text[4] = 'c';
    text[5] = ' ';
    text[6] = (char)('0' + ((music / 100U) % 10U));
    text[7] = (char)('0' + ((music / 10U) % 10U));
    text[8] = (char)('0' + (music % 10U));
    text[9] = '\0';
}

static uint8_t app_oled_string_len(const char *str)
{
    uint8_t len = 0U;

    if (str == NULL)
    {
        return 0U;
    }

    while ((str[len] != '\0') && (len < OLED_LINE_WIDTH))
    {
        len++;
    }

    return len;
}

static const char *app_oled_power_text(SysPower state)
{
    if (state != SYS_POWER_ON)
    {
        return "Power Off";
    }

    return (s_showPowerOn != 0U) ? "Power ON" : "CD Source";
}

static const char *app_oled_cd_text(SysState state)
{
    if (s_cdDisplay == (uint8_t)OLED_CD_DISPLAY_FAST_PREVIOUS)
    {
        return "Fast Previousing";
    }
    if (s_cdDisplay == (uint8_t)OLED_CD_DISPLAY_FAST_NEXT)
    {
        return "Fast Nexting";
    }

    switch (state)
    {
    case SYS_STATE_NO_DISC:
        return "No Disc";

    case SYS_STATE_LOADING:
        return "Loading";

    case SYS_STATE_EJECTING:
        return "Ejecting";

    case SYS_STATE_STOP:
        return "Stop";

    case SYS_STATE_PLAY:
        return "Play";

    case SYS_STATE_PAUSE:
        return "Pause";

    case SYS_STATE_POWER_OFF:
    default:
        return "CD Source";
    }
}
