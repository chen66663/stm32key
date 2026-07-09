#ifndef BSP_OLED_IIC_H
#define BSP_OLED_IIC_H

#include <stdint.h>
#include "com_err.h"

err_t bsp_oled_iic_init(void);
void bsp_oled_iic_clear(void);
void bsp_oled_iic_show_string(uint8_t x, uint8_t y, const char *str);
void bsp_oled_iic_show_num(uint8_t x, uint8_t y, uint32_t num, uint8_t len);
void bsp_oled_iic_show_bitmap(uint8_t x,
                              uint8_t page,
                              uint8_t width,
                              uint8_t pageCount,
                              const uint8_t *bitmap);
void bsp_oled_iic_show_chinese16(uint8_t x, uint8_t page, const uint8_t *font16x16);
void bsp_oled_iic_show_frame(const uint8_t *frame);
void bsp_oled_iic_refresh(void);

#endif /* BSP_OLED_IIC_H */
