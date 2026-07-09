#ifndef APP_TYPES_H
#define APP_TYPES_H

#include <stdint.h>

typedef enum
{
    MODULE_NONE = 0,
    MODULE_KEY,
    MODULE_POWER,
    MODULE_CD,
    MODULE_OLED,
    MODULE_LED
} ModuleId;

typedef enum
{
    MSG_ID_NONE = 0,
    MSG_ID_KEY_EVENT,
    MSG_ID_POWER_STATE,
    MSG_ID_CD_EVENT,
    MSG_ID_CD_STATE,
    MSG_ID_OLED_POWER,
    MSG_ID_OLED_REFRESH,
    MSG_ID_LED_EVENT
} MsgId;

typedef enum
{
    EV_KEY_WK_UP_SHORT = 0,
    EV_KEY_WK_UP_LONG,
    EV_KEY_WK_UP_OFF,
    EV_KEY_0_SHORT,
    EV_KEY_0_LONG,
    EV_KEY_0_OFF,
    EV_KEY_1_SHORT,
    EV_KEY_1_LONG,
    EV_KEY_1_OFF,
    EV_KEY_COUNT
} KeyEvent;

typedef enum
{
    KEY_TIMER_WKUP = 0,
    KEY_TIMER_KEY0,
    KEY_TIMER_KEY1,
    KEY_TIMER_COUNT
} KeyTimerId;

typedef enum
{
    SYS_POWER_OFF = 0,
    SYS_POWER_ON,
    SYS_POWER_COUNT
} SysPower;

typedef enum
{
    POWER_EVENT_KEY_ON = 0,
    POWER_EVENT_KEY_OFF,
    POWER_EVENT_ERROR,
    POWER_EVENT_TOGGLE,
    POWER_EVENT_COUNT,
    POWER_EVENT_NONE = POWER_EVENT_COUNT
} PowerEvent;

typedef enum
{
    SYS_STATE_POWER_OFF = 0,
    SYS_STATE_NO_DISC,
    SYS_STATE_LOADING,
    SYS_STATE_EJECTING,
    SYS_STATE_STOP,
    SYS_STATE_PLAY,
    SYS_STATE_PAUSE,
    SYS_STATE_COUNT
} SysState;

typedef enum
{
    MATRIX_EVENT_POWER_ON = 0,
    MATRIX_EVENT_POWER_OFF,
    MATRIX_EVENT_LOAD_EJECT,
    MATRIX_EVENT_DISC_ACTION_DONE,
    MATRIX_EVENT_PLAY_PAUSE,
    MATRIX_EVENT_PREVIOUS,
    MATRIX_EVENT_NEXT,
    MATRIX_EVENT_KEY_RELEASE,
    MATRIX_EVENT_TIMER_DISC_DONE,
    MATRIX_EVENT_TIMER_REPEAT,
    MATRIX_EVENT_COUNT,
    MATRIX_EVENT_NONE = MATRIX_EVENT_COUNT
} MatrixEvent;

typedef enum
{
    OLED_STATE_OFF = 0,
    OLED_STATE_READY,
    OLED_STATE_UPDATE,
    OLED_STATE_ERROR,
    OLED_STATE_COUNT
} OledState;

typedef enum
{
    OLED_EVENT_POWER_OFF = 0,
    OLED_EVENT_POWER_ON,
    OLED_EVENT_CD_UPDATE,
    OLED_EVENT_REFRESH,
    OLED_EVENT_COUNT,
    OLED_EVENT_NONE = OLED_EVENT_COUNT
} OledEvent;

typedef enum
{
    OLED_CD_DISPLAY_NORMAL = 0,
    OLED_CD_DISPLAY_FAST_PREVIOUS,
    OLED_CD_DISPLAY_FAST_NEXT,
    OLED_CD_DISPLAY_COUNT
} OledCdDisplay;

typedef struct
{
    ModuleId    srcModule;
    ModuleId    dstModule;
    MsgId       msgId;
    uint8_t     opt0;
    uint8_t     opt1;
    uint8_t     opt2;
    uint16_t    value;
} AppMsg;

typedef struct
{
    ModuleId    srcModule;
    ModuleId    dstModule;
    MsgId       msgId;
    SysPower    powerState;
    SysState    cdState;
    uint8_t     cdDisplay;
    uint16_t    music;
} OledMail;

#endif /* APP_TYPES_H */
