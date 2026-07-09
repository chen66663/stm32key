#ifndef BSP_GPIO_H
#define BSP_GPIO_H

#include <stdint.h>
#include "RTE_Components.h"
#include CMSIS_device_header

#define BSP_GPIO_MODE_OUTPUT_50M_PP 0x3U
#define BSP_GPIO_MODE_OUTPUT_50M_OD 0x7U
#define BSP_GPIO_MODE_INPUT_FLOAT 0x4U
#define BSP_GPIO_MODE_INPUT_PULL 0x8U
#define BSP_GPIO_MODE_AF_50M_PP 0xBU
#define BSP_GPIO_MODE_OUTPUT_PP_2MHZ 0x2U
#define BSP_GPIO_MODE_OUTPUT_OD_2MHZ 0x6U

void bsp_gpio_enable_clock(GPIO_TypeDef *port);
void bsp_gpio_set_config(GPIO_TypeDef *port, uint32_t pin, uint32_t config);
void bsp_gpio_write(GPIO_TypeDef *port, uint32_t pin, uint8_t high);
uint8_t bsp_gpio_read(GPIO_TypeDef *port, uint32_t pin);

#endif /* BSP_GPIO_H */
