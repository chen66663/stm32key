#ifndef BSP_GPIO_H
#define BSP_GPIO_H

/*----------------------------------------------------------------------------
 * BSP 底层 GPIO 驱动
 * 说明：封装 LED、按键、IIC 引脚的初始化与读写，业务层禁止直接操作寄存器
 *---------------------------------------------------------------------------*/

#include "com_type.h"
#include "com_config.h"

/* GPIO 电平状态 */
typedef enum
{
    BSP_GPIO_LOW = 0,
    BSP_GPIO_HIGH
} BspGpioLevel_E;

/* 初始化 */
void bsp_gpio_init(void);

/* LED 控制 */
void bsp_gpio_led_set(uint8_t ledId, BspGpioLevel_E level);
void bsp_gpio_led_toggle(uint8_t ledId);

/* 按键电平读取 */
BspGpioLevel_E bsp_gpio_key_read(uint8_t keyId);

#endif /* BSP_GPIO_H */
