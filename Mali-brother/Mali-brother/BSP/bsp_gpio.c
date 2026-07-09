#include "bsp_gpio.h"

/*----------------------------------------------------------------------------
 * BSP 底层 GPIO 驱动实现
 *---------------------------------------------------------------------------*/

void bsp_gpio_init(void)
{
    /* TODO: 配置 LED0/LED1 输出、WK_UP/KEY0/KEY1 输入、IIC 引脚 */
}

void bsp_gpio_led_set(uint8_t ledId, BspGpioLevel_E level)
{
    /* TODO: 按 ledId 设置对应 LED 引脚电平 */
    (void)ledId;
    (void)level;
}

void bsp_gpio_led_toggle(uint8_t ledId)
{
    /* TODO: 按 ledId 翻转对应 LED 引脚电平 */
    (void)ledId;
}

BspGpioLevel_E bsp_gpio_key_read(uint8_t keyId)
{
    /* TODO: 按 keyId 读取对应按键引脚电平 */
    (void)keyId;
    return BSP_GPIO_HIGH;
}
