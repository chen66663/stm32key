#ifndef COM_CONFIG_H
#define COM_CONFIG_H

/*----------------------------------------------------------------------------
 * 全局系统配置宏定义
 * 说明：集中管理任务栈、队列长度、消抖时长、长短按阈值、硬件引脚等
 *       禁止在业务/驱动代码中硬编码上述参数
 *---------------------------------------------------------------------------*/

#include "com_type.h"
#include "cmsis_os2.h"     /* 固定使用 CMSIS-RTOS2 / RTX5，优先级宏 osPriorityXxx 依赖此头文件 */

/* ==================== RTOS 任务栈大小（字节） ==================== */
#define TASK_KEY_STACK_SIZE         (512U)
#define TASK_POWER_STACK_SIZE       (512U)
#define TASK_CD_STACK_SIZE          (512U)
#define TASK_OLED_STACK_SIZE        (512U)

/* ==================== RTOS 任务优先级 ==================== */
#define TASK_KEY_PRIORITY           (osPriorityNormal)
#define TASK_POWER_PRIORITY         (osPriorityAboveNormal)
#define TASK_CD_PRIORITY            (osPriorityNormal)
#define TASK_OLED_PRIORITY          (osPriorityBelowNormal)

/* ==================== RTOS 消息队列长度 ==================== */
#define QUEUE_CD_LEN                (8U)
#define QUEUE_OLED_LEN              (8U)
#define QUEUE_POWER_LEN             (8U)

/* ==================== 按键参数 ==================== */
#define KEY_DEBOUNCE_MS             (30U)       /* 消抖时长 */
#define KEY_LONG_PRESS_MS           (1700U)     /* 长短按阈值 */
#define KEY_SCAN_PERIOD_MS          (10U)       /* 按键扫描周期 */

/* ==================== CD 播放器参数 ==================== */
#define CD_MUSIC_MIN                (1U)        /* 最小曲目编号 */
#define CD_MUSIC_MAX                (100U)      /* 最大曲目编号 */
#define CD_LONG_SWITCH_MS           (500U)      /* 长按连续切歌间隔 */

/* ==================== 硬件引脚定义 ==================== */
/* OLED IIC */
#define OLED_SCL_PIN                (GPIO_Pin_6)
#define OLED_SDA_PIN                (GPIO_Pin_7)
#define OLED_IIC_PORT               (GPIOB)

/* 按键 */
#define KEY_WKUP_PIN                (GPIO_Pin_0)
#define KEY0_PIN                    (GPIO_Pin_4)
#define KEY1_PIN                    (GPIO_Pin_3)

/* LED */
#define LED0_PIN                    (GPIO_Pin_8)    /* PA8 */
#define LED0_PORT                   (GPIOA)
#define LED1_PIN                    (GPIO_Pin_2)    /* PD2 */
#define LED1_PORT                   (GPIOD)

#endif /* COM_CONFIG_H */
