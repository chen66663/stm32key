#ifndef BSP_OLED_IIC_H
#define BSP_OLED_IIC_H

/*----------------------------------------------------------------------------
 * OLED IIC 板级驱动（BSP 层，G1 负责）
 * 说明：IIC 总线时序 + OLED 器件命令/显存/字库，合并为一个板级驱动
 *       BSP 层禁止调用 RTOS 延时/任务切换接口，仅使用 BSP 阻塞毫秒延时
 *---------------------------------------------------------------------------*/

#include "com_type.h"
#include "com_err.h"
#include "com_config.h"

/* ==================== 初始化 ==================== */
err_t bsp_oled_iic_init(void);

/* ==================== 显示操作 ==================== */
void bsp_oled_iic_clear(void);
void bsp_oled_iic_show_string(uint8_t x, uint8_t y, char *str);
void bsp_oled_iic_show_num(uint8_t x, uint8_t y, uint32_t num, uint8_t len);
void bsp_oled_iic_refresh(void);

#endif /* BSP_OLED_IIC_H */
