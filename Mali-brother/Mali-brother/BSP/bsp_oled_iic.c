/*----------------------------------------------------------------------------
 * OLED IIC 板级驱动实现（BSP 层，G1 负责）
 * 说明：本文件仅提供函数骨架，IIC 时序与 OLED 显示逻辑由 G1 实现
 *       IIC 引脚直接操作 BSP GPIO，禁止调用任何 RTOS 接口
 *       延时仅使用 BSP 阻塞毫秒延时
 *---------------------------------------------------------------------------*/

#include "bsp_oled_iic.h"
#include "bsp_gpio.h"

/* ==================== 模块私有静态变量 ==================== */
/* TODO(G1): 显示缓冲区等私有状态 */

/* ==================== 私有函数声明 ==================== */
/* TODO(G1): IIC 起始/停止/写字节/写命令/写数据等底层时序函数 */

/* ==================== 接口实现 ==================== */
err_t bsp_oled_iic_init(void)
{
    /* TODO(G1): IIC 引脚初始化 + OLED 上电初始化序列 */
    return ERR_OK;
}

void bsp_oled_iic_clear(void)
{
    /* TODO(G1): 清屏 */
}

void bsp_oled_iic_show_string(uint8_t x, uint8_t y, char *str)
{
    /* TODO(G1): 指定坐标显示字符串 */
    (void)x;
    (void)y;
    (void)str;
}

void bsp_oled_iic_show_num(uint8_t x, uint8_t y, uint32_t num, uint8_t len)
{
    /* TODO(G1): 指定坐标显示数值（用于 Music 001~100） */
    (void)x;
    (void)y;
    (void)num;
    (void)len;
}

void bsp_oled_iic_refresh(void)
{
    /* TODO(G1): 缓冲区刷新到屏幕 */
}
