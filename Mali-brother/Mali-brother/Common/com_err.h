#ifndef COM_ERR_H
#define COM_ERR_H

/*----------------------------------------------------------------------------
 * 全局错误码定义
 * 说明：统一项目内错误码，便于跨模块返回值判定
 *---------------------------------------------------------------------------*/

#include "com_type.h"

/* 错误码类型 */
typedef int16_t err_t;

/* 通用错误码 */
#define ERR_OK                  (0)     /* 成功 */
#define ERR_FAIL                (-1)    /* 通用失败 */
#define ERR_PARAM               (-2)    /* 参数非法 */
#define ERR_TIMEOUT             (-3)    /* 超时 */
#define ERR_NULL_PTR            (-4)    /* 空指针 */

/* 模块错误码（按模块分段预留） */
#define ERR_OLED_INIT_FAIL      (-100)  /* OLED 初始化失败 */
#define ERR_KEY_INIT_FAIL       (-110)  /* 按键初始化失败 */
#define ERR_CD_INIT_FAIL        (-120)  /* CD 状态机初始化失败 */
#define ERR_POWER_INIT_FAIL     (-130)  /* 电源管理初始化失败 */
#define ERR_RTOS_CREATE_FAIL    (-140)  /* RTOS 资源创建失败 */

#endif /* COM_ERR_H */
