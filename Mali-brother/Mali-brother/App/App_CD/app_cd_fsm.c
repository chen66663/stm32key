/*----------------------------------------------------------------------------
 * CD 播放器状态机实现（G3 私有业务）
 * 说明：两个矩阵与查表派发逻辑已按设计文档 4.2.3 / 4.2.4 实现完成；
 *       动作函数（app_cd_action_xxx）仅为骨架，内部业务逻辑由 G3 填充。
 *       - g_stateMatrix：状态迁移矩阵（当前状态 + 事件 -> 目标状态）
 *       - s_actionMatrix：动作函数矩阵（当前状态 + 事件 -> 动作函数）
 *       两个矩阵均排成 X/Y 网格：行 = 当前状态（左侧标签），列 = 输入事件
 *       （顶部列头），单元格按位置填全名。列顺序必须与 MatrixEvent 枚举一致。
 *---------------------------------------------------------------------------*/

#include "app_cd_fsm.h"
#include "com_config.h"

/* ==================== 模块私有状态 ==================== */
/* TODO(G3): 当前状态、当前曲目编号、是否有碟、CD Last 状态、切歌计时等 */
static SysState     s_curState;
static uint16_t         s_curMusic;
static SysState     s_lastState;        /* 掉电保存的 CD Last 状态 */

/* ==================== 动作函数声明（与文档动作矩阵一一对应） ==================== */
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

/* ==================== 状态迁移矩阵 ==================== */
/* 依据设计文档 4.2.3：g_stateMatrix[当前状态][事件] = 目标状态。
 *
 * X/Y 网格：行（Y）= 当前状态，见左侧标签；列（X）= 输入事件，见顶部列头。
 * 单元格 = 该状态遇该事件后迁移到的目标状态；保持原状态的格直接填本状态。
 *
 * 列顺序严格等于 MatrixEvent 枚举顺序（POWER_ON..KEY_RELEASE），按位置对应，
 * 改动事件枚举必须同步调整列。POWER_OFF 行 POWER_ON 列默认 STOP，运行时由
 * app_cd_action_power_on 依据 s_lastState 改写为 CD Last 状态。
 */
static const SysState g_stateMatrix[SYS_STATE_COUNT][MATRIX_EVENT_COUNT] =
{
/* 当前状态  \  事件:     POWER_ON              POWER_OFF            LOAD_EJECT           DISC_ACTION_DONE     PLAY_PAUSE           PREVIOUS             NEXT                 KEY_RELEASE          */
/* POWER_OFF */ {         SYS_STATE_STOP,       SYS_STATE_POWER_OFF, SYS_STATE_POWER_OFF, SYS_STATE_POWER_OFF, SYS_STATE_POWER_OFF, SYS_STATE_POWER_OFF, SYS_STATE_POWER_OFF, SYS_STATE_POWER_OFF },
/* NO_DISC   */ {         SYS_STATE_NO_DISC,    SYS_STATE_POWER_OFF, SYS_STATE_LOADING,   SYS_STATE_NO_DISC,   SYS_STATE_NO_DISC,   SYS_STATE_NO_DISC,   SYS_STATE_NO_DISC,   SYS_STATE_NO_DISC   },
/* LOADING   */ {         SYS_STATE_LOADING,    SYS_STATE_POWER_OFF, SYS_STATE_LOADING,   SYS_STATE_STOP,      SYS_STATE_LOADING,   SYS_STATE_LOADING,   SYS_STATE_LOADING,   SYS_STATE_LOADING   },
/* EJECTING  */ {         SYS_STATE_EJECTING,   SYS_STATE_POWER_OFF, SYS_STATE_EJECTING,  SYS_STATE_NO_DISC,   SYS_STATE_EJECTING,  SYS_STATE_EJECTING,  SYS_STATE_EJECTING,  SYS_STATE_EJECTING  },
/* STOP      */ {         SYS_STATE_STOP,       SYS_STATE_POWER_OFF, SYS_STATE_EJECTING,  SYS_STATE_STOP,      SYS_STATE_PLAY,      SYS_STATE_STOP,      SYS_STATE_STOP,      SYS_STATE_STOP      },
/* PLAY      */ {         SYS_STATE_PLAY,       SYS_STATE_POWER_OFF, SYS_STATE_EJECTING,  SYS_STATE_PLAY,      SYS_STATE_PAUSE,     SYS_STATE_PLAY,      SYS_STATE_PLAY,      SYS_STATE_PLAY      },
/* PAUSE     */ {         SYS_STATE_PAUSE,      SYS_STATE_POWER_OFF, SYS_STATE_EJECTING,  SYS_STATE_PAUSE,     SYS_STATE_PLAY,      SYS_STATE_PAUSE,     SYS_STATE_PAUSE,     SYS_STATE_PAUSE     },
};

/* ==================== 动作函数矩阵 ==================== */
/* 依据设计文档 4.2.4：s_actionMatrix[当前状态][事件] = 动作函数（无动作填 NULL）。
 *
 * 与状态矩阵同构的 X/Y 网格：行 = 当前状态，列 = 输入事件，列顺序同上。
 * NULL = 该状态对该事件无动作，dispatch 中对 NULL 判空跳过。
 */
static const CdStateAction s_actionMatrix[SYS_STATE_COUNT][MATRIX_EVENT_COUNT] =
{
/* 当前状态  \  事件:     POWER_ON                 POWER_OFF                LOAD_EJECT               DISC_ACTION_DONE          PLAY_PAUSE               PREVIOUS                 NEXT                     KEY_RELEASE                */
/* POWER_OFF */ {         app_cd_action_power_on,  NULL,                    NULL,                    NULL,                     NULL,                    NULL,                    NULL,                    NULL                       },
/* NO_DISC   */ {         NULL,                    app_cd_action_power_off, app_cd_action_load,      NULL,                     NULL,                    NULL,                    NULL,                    NULL                       },
/* LOADING   */ {         NULL,                    app_cd_action_power_off, NULL,                    app_cd_action_load_done,  NULL,                    NULL,                    NULL,                    NULL                       },
/* EJECTING  */ {         NULL,                    app_cd_action_power_off, NULL,                    app_cd_action_eject_done, NULL,                    NULL,                    NULL,                    NULL                       },
/* STOP      */ {         NULL,                    app_cd_action_power_off, app_cd_action_eject,     NULL,                     app_cd_action_play,      NULL,                    NULL,                    NULL                       },
/* PLAY      */ {         NULL,                    app_cd_action_power_off, app_cd_action_eject,     NULL,                     app_cd_action_pause,     app_cd_action_previous,  app_cd_action_next,      NULL                       },
/* PAUSE     */ {         NULL,                    app_cd_action_power_off, app_cd_action_eject,     NULL,                     app_cd_action_play,      app_cd_action_previous,  app_cd_action_next,      app_cd_action_key_release  },
};

/* ==================== 对外接口实现 ==================== */
err_t app_cd_fsm_init(void)
{
    /* 复位状态机：上电前停在 POWER_OFF，曲目从最小编号起，
     * CD Last 默认 STOP，待 power_on 动作按历史改写 */
    s_curState  = SYS_STATE_POWER_OFF;
    s_curMusic  = CD_MUSIC_MIN;
    s_lastState = SYS_STATE_STOP;
    return ERR_OK;
}

void app_cd_fsm_dispatch(MatrixEvent event)
{
    CdStateAction action;

    /* 1. 越界保护：MATRIX_EVENT_NONE == MATRIX_EVENT_COUNT，在矩阵维度之外，
     *    无效事件 / 无效状态直接忽略，避免数组越界访问 */
    if ((event >= MATRIX_EVENT_COUNT) || (s_curState >= SYS_STATE_COUNT))
    {
        return;
    }

    /* 2. 先执行动作（更新曲目编号、计时、保存/恢复 CD Last 等），无动作处为 NULL */
    action = s_actionMatrix[s_curState][event];
    if (action != NULL)
    {
        action();
    }

    /* 3. 再迁移状态：当前状态 + 事件 -> 目标状态 */
    s_curState = g_stateMatrix[s_curState][event];
}

SysState app_cd_fsm_get_state(void)
{
    return s_curState;
}

uint16_t app_cd_fsm_get_music(void)
{
    return s_curMusic;
}

/* ==================== 动作函数实现（骨架） ==================== */
static void app_cd_action_power_on(void)
{
    /* TODO(G3): 恢复 CD Last 状态 s_lastState */
}

static void app_cd_action_power_off(void)
{
    /* TODO(G3): 保存当前状态到 s_lastState */
}

static void app_cd_action_load(void)
{
    /* TODO(G3): 启动 3s 装碟计时 */
}

static void app_cd_action_load_done(void)
{
    /* TODO(G3): 装碟完成，曲目复位到 Music 001 */
}

static void app_cd_action_eject(void)
{
    /* TODO(G3): 启动 3s 退碟计时 */
}

static void app_cd_action_eject_done(void)
{
    /* TODO(G3): 退碟完成 */
}

static void app_cd_action_play(void)
{
    /* TODO(G3): 进入播放 */
}

static void app_cd_action_pause(void)
{
    /* TODO(G3): 进入暂停 */
}

static void app_cd_action_previous(void)
{
    /* TODO(G3): 上一曲，1 回绕到 100 */
}

static void app_cd_action_next(void)
{
    /* TODO(G3): 下一曲，100 回绕到 1 */
}

static void app_cd_action_key_release(void)
{
    /* TODO(G3): 结束 PAUSE 下的连续切歌 */
}
