#ifndef BSP_KEY_H
#define BSP_KEY_H

#include <stdint.h>

typedef enum
{
    BSP_KEY_WKUP = 0,
    BSP_KEY_0,
    BSP_KEY_1,
    BSP_KEY_COUNT
} BspKeyId;

void bsp_key_init(void);
uint8_t bsp_key_read(BspKeyId key);

#endif /* BSP_KEY_H */
