# 按键到 OLED 显示数据链路

本文档说明一次按键操作从按下、识别、消息传递、状态机处理，到最终 OLED 显示更新的完整数据链路。

以下以 `KEY1` 短按触发播放/暂停为例。

## 1. 按键按下

KEY 线程每 `10ms` 扫描一次按键。

```text
KEY1 被按下
```

KEY1 为低电平有效，底层 `bsp_key_read(BSP_KEY_1)` 会把低电平转换成 `1`，表示按键处于按下状态。

KEY 线程连续检测到 `3` 次有效按下后，确认按键有效。

```text
10ms 扫描周期 × 3 次 = 30ms 消抖
```

此时只是确认按键按下，还没有立即产生短按事件。短按事件需要等按键松开后才能确定。

## 2. KEY 模块生成按键事件

如果 KEY1 按下时间没有达到 `1.7s`，松开时 KEY 模块判断为短按。

```text
KEY1 松开
    -> EV_KEY_1_SHORT
```

KEY 模块根据事件表生成普通消息 `AppMsg`：

```text
srcModule = MODULE_KEY
dstModule = MODULE_CD
msgId     = MSG_ID_KEY_EVENT
value     = EV_KEY_1_SHORT
opt0      = 0
opt1      = 0
opt2      = 0
```

其中 `opt0`、`opt1`、`opt2` 当前没有使用，全部填 `0`，作为后续扩展字段保留。

## 3. KEY 发送消息到 CD 队列

KEY 模块通过普通 Message Queue 把消息发送给 CD 模块。

```text
KEY
    -> app_msg_send(g_cdMsgQueue, &msg)
    -> g_cdMsgQueue
    -> CD
```

`app_msg_send()` 内部流程：

```text
1. 从 g_appMsgPool 申请一个 AppMsg 内存块
2. 把当前消息内容复制到该内存块
3. 通过 osMessagePut() 把 AppMsg 指针放入 g_cdMsgQueue
```

这里普通 Message Queue 传递的是 `AppMsg` 指针，不是直接传整个结构体。

## 4. CD 线程接收按键消息

CD 线程阻塞等待自己的消息队列。

```c
msg = app_msg_get(g_cdMsgQueue, osWaitForever);
```

收到 KEY 发来的消息后，CD 根据 `msgId` 和 `value` 解析事件：

```text
MSG_ID_KEY_EVENT
value = EV_KEY_1_SHORT
```

然后转换为 CD 状态机事件：

```text
EV_KEY_1_SHORT
    -> MATRIX_EVENT_PLAY_PAUSE
```

## 5. CD 状态机处理事件

CD 状态机根据当前 CD 状态和事件查状态迁移矩阵。

如果当前是 `STOP`：

```text
SYS_STATE_STOP + MATRIX_EVENT_PLAY_PAUSE
    -> SYS_STATE_PLAY
```

如果当前是 `PLAY`：

```text
SYS_STATE_PLAY + MATRIX_EVENT_PLAY_PAUSE
    -> SYS_STATE_PAUSE
```

如果当前是 `PAUSE`：

```text
SYS_STATE_PAUSE + MATRIX_EVENT_PLAY_PAUSE
    -> SYS_STATE_PLAY
```

因此，KEY1 短按在 CD 模块中的作用就是播放和暂停切换。

## 6. CD 生成 OLED 显示数据

CD 状态变化后，会调用 `app_cd_report_state()` 生成 OLED 显示数据。

CD 不直接调用 OLED 函数，而是通过 Mail Queue 给 OLED 发送 `OledMail`。

`OledMail` 内容示例：

```text
srcModule  = MODULE_CD
dstModule  = MODULE_OLED
msgId      = MSG_ID_CD_STATE
powerState = SYS_POWER_ON
cdState    = SYS_STATE_PLAY 或 SYS_STATE_PAUSE
cdDisplay  = OLED_CD_DISPLAY_NORMAL
music      = 当前歌曲号
```

CD 发送 Mail 的流程：

```text
1. osMailAlloc(g_cdOledMailQueue, 0U)
2. 填充 OledMail 内容
3. osMailPut(g_cdOledMailQueue, mail)
```

数据流：

```text
CD
    -> g_cdOledMailQueue
    -> OLED
```

## 7. OLED 线程接收 Mail

OLED 线程周期性检查 CD 发来的 Mail Queue。

```c
evt = osMailGet(g_cdOledMailQueue, 0U);
```

收到 `OledMail` 后，OLED 更新自己的显示缓存：

```text
s_cdState   = mail->cdState
s_cdDisplay = mail->cdDisplay
s_music     = mail->music
```

然后触发 OLED 状态机事件：

```text
OLED_EVENT_CD_UPDATE
```

OLED 状态迁移：

```text
OLED_STATE_READY + OLED_EVENT_CD_UPDATE
    -> OLED_STATE_UPDATE
```

## 8. OLED 刷新显示

OLED 调用 `app_oled_render()` 重新绘制屏幕。

如果当前 Power 状态为 `SYS_POWER_ON`，OLED 显示：

```text
第 1 行：Power On
第 2 行：PLAY 或 PAUSE
第 3 行：Music ###
```

例如：

```text
Power On
PLAY
Music 001
```

最后调用：

```c
bsp_oled_iic_refresh();
```

数据通过软件 IIC 刷到 OLED。

当前 OLED 引脚配置：

```c
#define OLED_SCL_PORT GPIOB
#define OLED_SCL_PIN  6U
#define OLED_SDA_PORT GPIOB
#define OLED_SDA_PIN  7U
```

即：

```text
SCL = PB6
SDA = PB7
```

## 9. 完整链路

```text
KEY1 短按
    -> KEY 线程 10ms 扫描
    -> 30ms 消抖确认按下
    -> 松手后生成 EV_KEY_1_SHORT
    -> app_msg_send(g_cdMsgQueue, &msg)
    -> CD 线程收到 MSG_ID_KEY_EVENT
    -> EV_KEY_1_SHORT 转成 MATRIX_EVENT_PLAY_PAUSE
    -> CD 状态机完成 STOP/PLAY/PAUSE 迁移
    -> CD 生成 OledMail
    -> osMailPut(g_cdOledMailQueue, mail)
    -> OLED 线程 osMailGet()
    -> OLED 状态机进入 UPDATE
    -> app_oled_render()
    -> 软件 IIC PB6/PB7 刷新 OLED
```

一句话总结：

```text
按键
    -> KEY 线程
    -> CD Message Queue
    -> CD 状态机
    -> CD/OLED Mail Queue
    -> OLED 状态机
    -> OLED 显示
```

