#include "bsp_iwdg.h"
#include "com_config.h"
#include "RTE_Components.h"
#include CMSIS_device_header

void bsp_iwdg_init(void)
{
    IWDG->KR = 0x5555U;
    IWDG->PR = IWDG_PRESCALER_BITS;
    IWDG->RLR = IWDG_RELOAD_VALUE;
    IWDG->KR = 0xAAAAU;
    IWDG->KR = 0xCCCCU;
}

void bsp_iwdg_feed_dog(void)
{
    IWDG->KR = 0xAAAAU;
}
