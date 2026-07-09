# CLAUDE.md — STM32F103 RTOS CD播放器实训项目规范
## 文档说明
| 序号 | 说明内容 |
| ---- | -------- |
| 1 | 文件用途：Claude Code 项目全局约束文件，AI生成/修改/新增代码必须严格遵守本文全部规则 |
| 2 | 项目背景：东软汽车电子MCU实训，5人分组（G1/G2/G3/G4）开发CD按键响应嵌入式程序 |
| 3 | 技术栈：STM32F103、Keil MDK5、CMSIS-RTOS2 / RTX5（固定，业务直接调用 osXxx 原生 API）、IIC OLED、GPIO按键、独立看门狗 |
| 4 | 分层架构：BSP底层（含GPIO/RCC/IWDG/OLED IIC驱动）→ Middleware硬件无关中间件（预留）→ App业务线程三层架构，RTOS（RTX5）由 CMSIS 组件直接提供 |
| 5 | 约束优先级：目录分层 > 文件命名 > 代码命名 > 代码格式 > RTOS通信规则 |

---

# 一、项目整体业务&技术硬性需求
## 1. 业务功能需求
1. **线程规划**：四大独立RTOS线程（KEY按键线程、Power电源线程、CD模拟播放器线程、OLED显示线程）；
2. **硬件资源**：OLED IIC(PB6/PB7)、WK_UP/KEY0/KEY1按键、IWDG看门狗、LED0(PA8)/LED1(PD2)；
3. **按键逻辑**：30ms消抖判定按下、1.7s长短按阈值；WK_UP控制整机上下电，KEY0切碟/上一曲，KEY1播放暂停/下一曲；
4. **CD状态机**：上电恢复历史播放状态，共100首循环，长按连续切歌0.5s/首；
5. **OLED显示**：三行固定显示（第一行电源状态、第二行CD设备状态、第三行歌曲编号Music 001~100）；
6. **电源管理**：Power按键翻转整机状态，系统异常进入Power Off安全状态，必要时触发看门狗复位；
7. **增值功能**：LED区分上电/按键按下/抬起实现快闪、呼吸灯；

## 2. 技术强制约束
1. 分层解耦：业务代码禁止直接操作寄存器，仅调用BSP/Middleware API；OLED 等外部器件驱动归入 BSP 层；
2. 线程通信：跨模块业务通信仅允许通过RTOS消息队列传递结构体，禁止全局变量跨模块传业务状态；模块内部允许维护私有static状态；
3. RTOS固定：本项目固定使用 CMSIS-RTOS2 / Keil RTX5，业务层直接调用 `osXxx()` 原生 API（osThreadNew/osMessageQueueNew/osMessageQueuePut/osMessageQueueGet/osDelay 等），不再额外封装适配层；
4. 可扩展性：按键、电源消息预留3个8bit扩展opt参数；
5. 无硬编码：任务栈、队列长度、消抖时长、硬件引脚全部用宏定义；
6. 版本管理：代码、文档统一使用SVN管控。

---

# 二、标准工程目录结构（Keil MDK5）
```plaintext
Project_RTOS_CDPlayer/
├─ Core/ # 系统内核、应用编排层（全员协同开发）
│ ├─ main.c # 内核初始化、全局队列创建、四大任务创建入口
│ └─ main.h
├─ BSP/ # 全部板级硬件底层驱动（GPIO/时钟/看门狗/OLED）
│ ├─ bsp_gpio.c
│ ├─ bsp_gpio.h
│ ├─ bsp_rcc.c
│ ├─ bsp_rcc.h
│ ├─ bsp_iwdg.c # G4 开发：独立看门狗驱动
│ ├─ bsp_iwdg.h
│ ├─ bsp_oled_iic.c # G1 开发：IIC总线时序 + OLED器件驱动（合并）
│ └─ bsp_oled_iic.h
├─ Middleware/ # 硬件无关通用中间件（预留：状态机查表、消息封装工具等）
├─ App/ # 业务线程层（按分组拆分开发）
│ ├─ App_OLED/ # G1：OLED 显示业务线程
│ │ ├─ app_oled_task.c
│ │ └─ app_oled_task.h
│ ├─ App_KEY/ # G2：按键扫描业务线程
│ │ ├─ app_key_task.c
│ │ └─ app_key_task.h
│ ├─ App_CD/ # G3：CD 播放器状态机线程
│ │ ├─ app_cd_task.c # 线程入口：收 AppMsg、调 FSM 派发、回报 OLED
│ │ ├─ app_cd_task.h
│ │ ├─ app_cd_fsm.c # 状态机：状态迁移矩阵 + 动作函数矩阵（G3 私有）
│ │ └─ app_cd_fsm.h
│ └─ App_Power/ # G4：电源管理线程
│ ├─ app_power_task.c
│ └─ app_power_task.h
├─ Common/ # 全局通用公共文件（全模块依赖）
│ ├─ com_type.h
│ ├─ com_err.h
│ ├─ app_types.h # 统一消息载体AppMsg + 模块/消息ID + CD状态/事件枚举（核心）
│ └─ com_config.h
├─ CMSIS/ # Keil自带CMSIS库、STM32F103标准库、RTOS相关内核或适配文件
└─ User_Doc/ # 项目配套文档存放目录
├─ Design/
├─ Test/
├─ Record/
└─ Daily_Report/
```

### 分组开发目录权责
| 分组 | 负责目录                          | 开发内容                                         | 难度   |
| ---- | --------------------------------- | ------------------------------------------------ | ------ |
| G1   | BSP/bsp_oled_iic + App/App_OLED   | OLED IIC底层驱动（总线+器件） + OLED显示业务线程 | 中+易  |
| G2   | App/App_KEY                       | 按键扫描、消抖、长短按识别、按键消息发送         | 难     |
| G3   | App/App_CD                        | CD状态机（矩阵迁移表+动作函数矩阵，独立 app_cd_fsm.c）、切歌/切碟逻辑、CD消息收发 | 难     |
| G4   | App/App_Power + BSP/bsp_iwdg      | 电源管理线程、整机上下电逻辑、看门狗驱动         | 易+中  |
| 全体共用 | Core/、Common/、BSP/(gpio/rcc)  | 工程初始化、RTOS封装、全局公共类型/消息、系统配置 | 基础作业 |

---

# 三、文件命名强制规范
## 通用规则
1. 所有文件名称**全部小写**，单词下划线`_`分隔；
2. 目录名称按照“标准工程目录结构”固定使用，新增目录优先使用小写下划线；文件禁止驼峰、大写字母、中文、特殊符号；
3. `.c`源文件与`.h`头文件成对命名，前缀完全一致；
4. 按分层固定后缀区分驱动、业务、消息文件。

## 分层命名模板
| 分层类型       | 命名规则               | 示例                  |
| -------------- | ---------------------- | --------------------- |
| BSP底层驱动    | `bsp_外设名.c/h`       | bsp_gpio.c、bsp_iwdg.h、bsp_oled_iic.c |
| 业务线程文件   | `app_模块名_task.c/h`  | app_key_task.c、app_cd_task.h |
| 模块状态机文件 | `app_模块名_fsm.c/h`（模块私有，不对外开放） | app_cd_fsm.c |
| 全局统一类型   | `app_types.h`（统一消息体 AppMsg + 模块/消息/状态/事件枚举集中定义，仅头文件） | app_types.h |
| 全局公共文件   | `com_功能名.h`         | com_type.h、com_err.h |

---

# 四、代码命名强制规范
## 1. 函数命名规则
| 函数类型               | 命名规则                  | 示例代码                                      |
| ---------------------- | ------------------------- | --------------------------------------------- |
| BSP底层硬件函数        | `bsp_外设_动作()`         | `void bsp_gpio_init(void);` <br> `void bsp_iwdg_feed_dog(void);` |
| OLED驱动函数           | `bsp_oled_iic_动作()`         | `void bsp_oled_iic_show_string(uint8_t x, uint8_t y, char *str);` |
| RTOS任务入口           | `app_模块名_taskEntry(void *arg)` | `void app_cd_taskEntry(void *arg);` |
| 模块私有消息处理函数   | `static void app_模块名_msgHandle()` | - |
| 状态机动作函数         | `static void app_cd_action_动作()`   | `app_cd_action_play`、`app_cd_action_next` |
| 状态机派发/查表接口    | `app_模块名_fsm_动作()`               | `app_cd_fsm_dispatch(event);` |
| RTOS原生接口           | 直接使用 CMSIS-RTOS2（`osXxx()`）  | `osDelay(10);`、`osMessageQueuePut(...)` |

## 2. 变量命名规则
| 变量类型       | 命名规则               | 示例                          |
| -------------- | ---------------------- | ----------------------------- |
| 局部变量       | 纯小驼峰，无前缀       | `uint16_t currentMusicIndex;` |
| 模块静态变量   | 前缀s_ + 小驼峰        | `static uint8_t s_systemPowerState;` |
| 跨模块全局句柄 | 前缀g_ + 小驼峰        | `extern osMessageQueueId_t g_sysMsgQueue;` |
| 枚举           | 大驼峰，无后缀         | `typedef enum {KEY_SHORT_PRESS, KEY_LONG_PRESS} KeyEvent;` |
| 结构体         | 大驼峰，无后缀         | `typedef struct {MatrixEvent eventType; uint8_t keyId;} KeyMsg;` |

## 3. 宏定义规范
- 命名规则：全大写 + 下划线；
- 分类示例：
  | 宏定义类型   | 示例                          |
  | ------------ | ----------------------------- |
  | 硬件引脚     | `#define OLED_SDA_PIN GPIO_PIN_7` |
  | RTOS配置     | `TASK_XXX_STACK_SIZE`、`QUEUE_XXX_LEN` |
  | 系统状态     | `SYS_POWER_ON`                |
  | 错误码       | `ERR_OLED_INIT_FAIL`          |

---

# 五、代码格式与头文件规范
## 代码格式
1. 缩进统一4个空格，禁用Tab；
2. 单行代码≤120字符；
3. 大括号`{}`独占一行，禁止行尾紧跟`{}`；
4. 运算符两侧加空格，逗号后加空格。

## 头文件规范
1. 头文件保护宏：文件名全大写，下划线拼接，示例：
```c
#ifndef APP_KEY_TASK_H
#define APP_KEY_TASK_H
// 代码声明
#endif /* APP_KEY_TASK_H */
```

依赖规则：
- 所有模块优先包含`com_type.h`、`com_config.h`；
- 业务层禁止直接调用寄存器库；
- 跨模块通信统一依赖`app_types.h`（统一消息体 AppMsg 与全部状态/事件枚举集中于此），不再使用分模块消息头文件；
- 需要 RTOS 接口（线程/队列/延时）的文件直接包含`cmsis_os2.h`，使用 `osXxx()` 原生 API；
- OLED 器件驱动归入 BSP 层（`bsp_oled_iic.c/h`），业务层经 App_OLED 线程间接刷新显示。

---

# 六、RTOS 与业务开发红线（禁止行为）
1. 禁止跨模块用全局变量传递业务状态，统一通过消息队列结构体通信；
2. 本项目固定使用 CMSIS-RTOS2 / RTX5，业务层与 Core 直接调用 `osXxx()` 原生 API（`osThreadNew`/`osMessageQueueNew`/`osDelay`/`osMessageQueuePut`/`osMessageQueueGet` 等）；不再保留 `system_rtos` 封装层；
3. 禁止代码内硬编码延时、栈大小、队列长度、按键消抖时间；
4. 禁止分散多处喂狗，看门狗逻辑统一放在Power线程循环；
5. 硬件驱动中禁止调用RTOS延时、任务切换接口，仅使用BSP阻塞毫秒延时；
6. 禁止修改目录分层结构，新增文件必须遵循命名规范。

