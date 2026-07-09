#ifndef APP_TYPES_H
#define APP_TYPES_H

/*----------------------------------------------------------------------------
 * 全局统一消息与状态类型定义（核心文件）
 * 说明：集中定义跨模块通信的统一消息载体 AppMsg、模块/消息 ID 与 CD 状态机枚举
 *       跨模块业务通信仅允许通过 RTOS 消息队列传递本文件定义的 AppMsg
 *       禁止用全局变量跨模块传递业务状态
 *       依据《CD播放器线程通信与状态机设计规划文档》第二/四/五章定义
 *---------------------------------------------------------------------------*/

#include "com_type.h"

/* ============================================================================
 * 一、模块标识（消息来源 / 目标）
 * ==========================================================================*/
typedef enum
{
    MODULE_NONE = 0,
    MODULE_KEY,             /* 按键线程（G2） */
    MODULE_POWER,           /* 电源线程（G4） */
    MODULE_CD,              /* CD 播放器线程（G3） */
    MODULE_OLED             /* OLED 显示线程（G1） */
} ModuleId;

/* ============================================================================
 * 二、消息类型 ID
 * ==========================================================================*/
typedef enum
{
    MSG_ID_NONE = 0,
    MSG_ID_POWER_STATE,     /* Power 状态通知（Power -> CD / OLED） */
    MSG_ID_CD_EVENT,        /* CD 控制事件（KEY / Power -> CD），value/opt 携带 MatrixEvent */
    MSG_ID_CD_STATE,        /* CD 状态 + 歌曲编号（CD -> OLED） */
    MSG_ID_OLED_REFRESH     /* OLED 刷新请求 */
} MsgId;

/* ============================================================================
 * 三、统一消息结构体 AppMsg
 *   - 所有跨线程消息队列（g_cdMsgQueue / g_oledMsgQueue / g_powerMsgQueue）
 *     统一传递本结构体
 *   - 字段含义随 msgId 变化，未用到的字段一律填 0
 *
 *   字段填法契约（依据设计文档 5.1）：
 *   ┌─────────────────────┬───────────┬──────────────┬──────────────┬──────┬──────┐
 *   │ msgId               │ 方向      │ value        │ opt0         │ opt1 │ opt2 │
 *   ├─────────────────────┼───────────┼──────────────┼──────────────┼──────┼──────┤
 *   │ MSG_ID_CD_EVENT     │ KEY/PWR→CD│ MatrixEvent  │ 0            │ 0    │ 0    │
 *   │ MSG_ID_CD_STATE     │ CD→OLED   │ 曲目号 1~100 │ SysState     │ 0    │ 0    │
 *   │ MSG_ID_POWER_STATE  │ PWR→OLED/CD│ SysPower    │ 0            │ 0    │ 0    │
 *   │ MSG_ID_OLED_REFRESH │ 任意→OLED │ 0            │ 刷新行掩码   │ 0    │ 0    │
 *   └─────────────────────┴───────────┴──────────────┴──────────────┴──────┴──────┘
 *   说明：
 *     - CD_EVENT 的事件号放 value，CD 线程取 (MatrixEvent)msg.value 喂 fsm_dispatch
 *     - CD_STATE 一条带全：value=曲目号、opt0=状态，OLED 收到即可同刷第二/第三行
 *     - srcModule/dstModule 始终填写，兼作路由与调试抓包
 * ==========================================================================*/
typedef struct
{
    ModuleId    srcModule;      /* 消息来源模块（始终填写） */
    ModuleId    dstModule;      /* 消息目标模块（始终填写） */
    MsgId       msgId;          /* 消息类型，决定下列字段如何解释 */
    uint8_t     opt0;           /* 扩展参数 0：CD_STATE 时承载 SysState，余预留 */
    uint8_t     opt1;           /* 扩展参数 1（预留，默认 0） */
    uint8_t     opt2;           /* 扩展参数 2（预留，默认 0） */
    uint16_t    value;          /* 主载荷：事件号 / 状态值 / 曲目号 */
} AppMsg;

/* ============================================================================
 * 四、CD 系统状态枚举（CD 产生，OLED 消费）
 *   - 设计文档 2.1 节
 * ==========================================================================*/
typedef enum
{
    SYS_STATE_POWER_OFF = 0,    /* 系统掉电 / CD 不响应 */
    SYS_STATE_NO_DISC,          /* 上电且无碟 */
    SYS_STATE_LOADING,          /* 装碟中（3s 时序） */
    SYS_STATE_EJECTING,         /* 退碟中（3s 时序） */
    SYS_STATE_STOP,             /* 有碟停止 */
    SYS_STATE_PLAY,             /* 播放 */
    SYS_STATE_PAUSE,            /* 暂停 */
    SYS_STATE_COUNT             /* 状态总数（矩阵维度，勿作为有效状态） */
} SysState;

/* ============================================================================
 * 五、CD 状态机事件枚举（KEY / Power 产生，CD 消费）
 *   - 设计文档 4.2.1 节，列顺序与状态迁移矩阵列序严格一致
 * ==========================================================================*/
typedef enum
{
    MATRIX_EVENT_POWER_ON = 0,      /* 系统上电（WK_UP 短按） */
    MATRIX_EVENT_POWER_OFF,         /* 系统掉电（WK_UP 长按 1.7s） */
    MATRIX_EVENT_LOAD_EJECT,        /* 装碟 / 退碟（KEY_0 短按） */
    MATRIX_EVENT_DISC_ACTION_DONE,  /* 装碟 / 退碟完成（内部 3s 计时） */
    MATRIX_EVENT_PLAY_PAUSE,        /* 播放 / 暂停（KEY_1 短按） */
    MATRIX_EVENT_PREVIOUS,          /* 上一曲（KEY_0 长按 1.7s） */
    MATRIX_EVENT_NEXT,              /* 下一曲（KEY_1 长按 1.7s） */
    MATRIX_EVENT_KEY_RELEASE,       /* 按键抬起（结束连续切歌） */
    MATRIX_EVENT_COUNT,             /* 事件总数（矩阵维度，勿作为有效事件） */
    MATRIX_EVENT_NONE = MATRIX_EVENT_COUNT  /* 无效事件 / 不响应 */
} MatrixEvent;

/* ============================================================================
 * 六、整机电源状态枚举（Power 维护）
 * ==========================================================================*/
typedef enum
{
    SYS_POWER_OFF = 0,
    SYS_POWER_ON
} SysPower;

#endif /* APP_TYPES_H */
