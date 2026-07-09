#ifndef APP_OLED_ANIM_H
#define APP_OLED_ANIM_H

#include <stdint.h>
#include "com_err.h"

#define OLED_BOOT_ANIM_FRAME_COUNT  48U
#define OLED_BOOT_ANIM_FRAME_DELAY  25U
#define OLED_BOOT_ANIM_FRAME_SIZE   1024U

err_t app_oled_play_boot_animation(void);

#endif /* APP_OLED_ANIM_H */
