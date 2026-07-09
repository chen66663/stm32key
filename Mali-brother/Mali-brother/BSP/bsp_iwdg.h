#ifndef BSP_IWDG_H
#define BSP_IWDG_H

/*----------------------------------------------------------------------------
 * BSP 独立看门狗驱动（G4 开发）
 * 说明：封装 IWDG 初始化与喂狗，喂狗逻辑统一由 Power 线程调用
 *---------------------------------------------------------------------------*/

#include "com_type.h"

/* 看门狗初始化（设置溢出周期） */
void bsp_iwdg_init(void);

/* 喂狗 */
void bsp_iwdg_feed_dog(void);

#endif /* BSP_IWDG_H */
