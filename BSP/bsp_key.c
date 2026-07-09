#include "bsp_key.h"
#include "bsp_gpio.h"

#define WKUP_PORT   GPIOA
#define WKUP_PIN    0U
#define KEY0_PORT   GPIOC
#define KEY0_PIN    1U
#define KEY1_PORT   GPIOC
#define KEY1_PIN    13U

void bsp_key_init(void)
{
    bsp_gpio_enable_clock(WKUP_PORT);
    bsp_gpio_set_config(WKUP_PORT, WKUP_PIN, BSP_GPIO_MODE_INPUT_PULL);
    bsp_gpio_write(WKUP_PORT, WKUP_PIN, 0U);

    bsp_gpio_enable_clock(KEY0_PORT);
    bsp_gpio_set_config(KEY0_PORT, KEY0_PIN, BSP_GPIO_MODE_INPUT_PULL);
    bsp_gpio_write(KEY0_PORT, KEY0_PIN, 1U);

    bsp_gpio_enable_clock(KEY1_PORT);
    bsp_gpio_set_config(KEY1_PORT, KEY1_PIN, BSP_GPIO_MODE_INPUT_PULL);
    bsp_gpio_write(KEY1_PORT, KEY1_PIN, 1U);
}

uint8_t bsp_key_read(BspKeyId key)
{
    switch (key)
    {
    case BSP_KEY_WKUP:
        return bsp_gpio_read(WKUP_PORT, WKUP_PIN);

    case BSP_KEY_0:
        return (bsp_gpio_read(KEY0_PORT, KEY0_PIN) == 0U) ? 1U : 0U;

    case BSP_KEY_1:
        return (bsp_gpio_read(KEY1_PORT, KEY1_PIN) == 0U) ? 1U : 0U;

    default:
        return 0U;
    }
}
