#ifndef COM_CONFIG_H
#define COM_CONFIG_H

#define APP_MSG_POOL_LEN            24U

#define QUEUE_CD_LEN                8U
#define QUEUE_POWER_LEN             8U
#define MAIL_OLED_LEN               8U

#define TASK_KEY_PRIORITY           osPriorityAboveNormal
#define TASK_POWER_PRIORITY         osPriorityNormal
#define TASK_CD_PRIORITY            osPriorityBelowNormal
#define TASK_OLED_PRIORITY          osPriorityLow

#define TASK_KEY_STACK_SIZE         512U
#define TASK_POWER_STACK_SIZE       512U
#define TASK_CD_STACK_SIZE          768U
#define TASK_OLED_STACK_SIZE        768U

#define KEY_SCAN_INTERVAL_MS        10U
#define KEY_PRESS_SAMPLES           3U
#define KEY_LONG_PRESS_MS           1700U
#define KEY_LONG_MARGIN_MS          80U
#define CD_REPEAT_INTERVAL_MS       500U
#define KEY_TIMER_MSG_PRIORITY      0U
#define CD_TIMER_MSG_PRIORITY       0U

#define CD_TASK_PERIOD_MS           50U
#define CD_DISC_ACTION_MS           3000U
#define CD_MUSIC_MIN                1U
#define CD_MUSIC_MAX                100U

#define OLED_TASK_PERIOD_MS         50U
#define OLED_POWER_ON_DISPLAY_MS    1000U
#define OLED_LINE_WIDTH             21U
#define OLED_IIC_ADDR               0x78U
#define OLED_WIDTH                  128U
#define OLED_PAGE_COUNT             8U
#define OLED_SCL_PORT               GPIOB
#define OLED_SCL_PIN                6U
#define OLED_SDA_PORT               GPIOB
#define OLED_SDA_PIN                7U

#define IWDG_RELOAD_VALUE           0x0FFFU
#define IWDG_PRESCALER_BITS         0x06U

#endif /* COM_CONFIG_H */
