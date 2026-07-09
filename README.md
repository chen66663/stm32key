# stm32key

基于 STM32F103RC 和 Keil MDK 的按键、状态机与 OLED 显示示例工程。工程使用 CMSIS-RTOS RTX 组织多任务，通过消息队列和 Mail 队列把按键、电源、CD 状态机和 OLED 显示模块解耦。

## 工程概览

- 主控: STM32F103RC, Cortex-M3, 72 MHz
- 工程文件: `chezai.uvprojx`
- 目标名: `Target 1`
- 输出名: `chezai`, 构建后生成 `Objects/chezai.hex`
- RTOS: CMSIS-RTOS RTX
- 显示: I2C OLED, 地址 `0x78`, SCL `PB6`, SDA `PB7`
- 按键: `WKUP` 位于 `PA0`, `KEY0` 位于 `PC1`, `KEY1` 位于 `PC13`

## 功能

- 三路按键扫描，支持短按、长按和释放事件。
- `WKUP` 短按开机，`WKUP` 长按关机。
- `KEY0` 短按执行装载或弹出，长按切换上一曲。
- `KEY1` 短按播放或暂停，长按切换下一曲。
- CD 状态机覆盖关机、无碟、装载、弹出、停止、播放、暂停等状态。
- OLED 根据电源状态、CD 状态和曲目编号刷新显示，并带有开机显示/动画入口。
- 独立看门狗初始化，降低任务异常卡死风险。

## 目录结构

```text
App/
  App_CD/       CD 状态机和 CD 任务
  App_KEY/      按键扫描、消抖、长按定时
  App_OLED/     OLED 显示任务和动画
  App_Power/    电源状态任务
BSP/            GPIO、按键、OLED I2C、IWDG 等板级驱动
Common/         公共类型、错误码、配置和消息封装
Core/           main 入口与任务、队列创建
RTE/            Keil RTE 与 CMSIS-RTOS 配置
DebugConfig/    Keil 调试配置
```

根目录下的中文 Markdown 文档记录了模块状态机、线程通信和按键到 OLED 的数据链路，适合配合源码阅读。

## 模块通信

`Core/main.c` 创建全局消息池、三个消息队列和一个 CD 到 OLED 的 Mail 队列:

- `g_powerMsgQueue`: 接收按键触发的电源事件。
- `g_cdMsgQueue`: 接收电源状态、按键事件和 CD 内部定时事件。
- `g_oledMsgQueue`: 接收电源状态和刷新事件。
- `g_cdOledMailQueue`: CD 任务向 OLED 任务上报状态、显示模式和曲目编号。

`Common/app_msg.c` 负责从消息池分配 `AppMsg`，投递到目标队列，并在消费后释放，避免各任务直接共享临时栈数据。

## 构建方法

1. 安装 Keil MDK 5，并确保包含 STM32F1 设备包和 ARM Compiler 5。
2. 使用 Keil uVision 打开 `chezai.uvprojx`。
3. 选择 `Target 1`。
4. 执行 Build 或 Rebuild。
5. 构建产物位于 `Objects/`，其中 `chezai.hex` 可用于烧录。

也可以在已配置 Keil 命令行环境的机器上使用类似命令构建:

```powershell
UV4.exe -b .\chezai.uvprojx -t "Target 1"
```

## 主要配置

常用参数集中在 `Common/com_config.h`:

- `KEY_SCAN_INTERVAL_MS`: 按键扫描周期，默认 10 ms。
- `KEY_PRESS_SAMPLES`: 按键确认采样次数，默认 3 次。
- `KEY_LONG_PRESS_MS`: 长按阈值，默认 1700 ms。
- `CD_DISC_ACTION_MS`: 装载/弹出模拟动作时间，默认 3000 ms。
- `CD_REPEAT_INTERVAL_MS`: 长按连跳间隔，默认 500 ms。
- `CD_MUSIC_MIN` / `CD_MUSIC_MAX`: 曲目范围，默认 1 到 100。

## 版本管理说明

仓库只提交源码、工程配置和设计文档。`Objects/`、`Listings/`、`*.hex`、`*.axf`、Keil 用户界面状态文件以及本地工具缓存会被 `.gitignore` 排除。
