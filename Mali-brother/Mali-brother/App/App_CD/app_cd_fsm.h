#ifndef APP_CD_FSM_H
#define APP_CD_FSM_H

/*----------------------------------------------------------------------------
 * CD 播放器状态机（G3 私有业务）
 * 说明：矩阵模型状态机的对外接口
 *       - 状态迁移矩阵 g_stateMatrix[当前状态][事件] -> 目标状态
 *       - 动作函数矩阵 s_actionMatrix[当前状态][事件] -> 动作函数
 *       仅供 App_CD 线程内部调用，不对其他模块开放
 *---------------------------------------------------------------------------*/

#include "com_type.h"
#include "com_err.h"
#include "app_types.h"

/* 动作函数指针类型：处理“当前状态 + 事件”对应的具体动作 */
typedef void (*CdStateAction)(void);

/* 状态机初始化（复位当前状态、曲目编号等私有数据） */
err_t app_cd_fsm_init(void);

/* 事件派发：依据当前状态与事件查表，执行动作并完成状态迁移 */
void app_cd_fsm_dispatch(MatrixEvent event);

/* 获取当前 CD 状态（供线程组装上报 OLED 的消息） */
SysState app_cd_fsm_get_state(void);

/* 获取当前歌曲编号 1~100（供线程组装上报 OLED 的消息） */
uint16_t app_cd_fsm_get_music(void);

#endif /* APP_CD_FSM_H */
