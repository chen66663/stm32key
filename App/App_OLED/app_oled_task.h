#ifndef APP_OLED_TASK_H
#define APP_OLED_TASK_H

#include "cmsis_os.h"
#include "com_err.h"

err_t app_oled_init(void);
void app_oled_taskEntry(void const *argument);
void app_oled_show_bitmap(uint8_t x,
                          uint8_t page,
                          uint8_t width,
                          uint8_t pageCount,
                          const uint8_t *bitmap);
void app_oled_show_chinese16(uint8_t x, uint8_t page, const uint8_t *font16x16);
void app_oled_show_mali_brother_group(uint8_t x, uint8_t page);

#endif /* APP_OLED_TASK_H */
