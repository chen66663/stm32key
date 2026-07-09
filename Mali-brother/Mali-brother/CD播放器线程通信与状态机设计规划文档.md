# CD播放器线程通信与状态机设计规划文档

## 文档说明

* 文档用途：明确CD播放器实训项目的线程架构、状态机实现、通信规则及功能模块划分，为嵌入式开发和测试提供设计依据

* 适用范围：STM32F103 MCU实训项目中的按键扫描、电源管理、CD模拟控制、OLED显示、看门狗和线程间通信开发

* 关联流程：系统上下电流程、按键长短按识别流程、CD装碟/退碟流程、播放/暂停/切歌流程、OLED状态显示流程

## 一、核心设计原则

### 1.1 架构设计原则

* 单一职责：各线程仅负责本模块功能，不跨模块直接修改对方状态

* 分层解耦：业务层禁止直接操作寄存器，只能调用BSP或中间件驱动接口

* 消息驱动：Key、Power、CD、OLED之间通过消息队列传递结构体消息，禁止跨模块全局变量传递业务状态

* 可扩展：Key Message和Power Message预留3个8bit Option参数，便于后续扩展按键、模式和状态

* 固定RTOS：本项目固定使用CMSIS-RTOS2 / RTX5，业务层与Core直接调用`osXxx()`原生API（`osThreadNew`/`osMessageQueueNew`/`osDelay`/`osMessageQueuePut`/`osMessageQueueGet`等），不再保留`system_rtos`封装层

### 1.2 核心技术原则

* 状态机实现：CD模块采用函数指针数组映射“状态+事件”对应动作函数

* 状态迁移：采用Matrix模型，通过“当前状态 + 输入事件”查二维表确定目标状态

* 事件驱动：按键短按、长按、抬起、电源状态变化、装碟/退碟完成均抽象为事件

* 时序控制：按键30ms消抖、1.7s长按判断、CD装碟/退碟3s完成、连续切歌0.5s/首均使用宏定义配置

## 二、系统状态定义

### 2.1 状态枚举分类

| 状态名称 | 状态描述 | 关联功能 |
| -------- | -------- | -------- |
| SYS_STATE_POWER_OFF | 系统掉电/CD不可响应状态 | CD不响应除Power On外的其他按键，OLED显示Power Off |
| SYS_STATE_NO_DISC | 上电且无碟状态 | OLED第二行显示NO DISC，Load/Eject短按触发Load流程 |
| SYS_STATE_LOADING | 装碟中状态 | OLED第二行显示LOADING，3s后迁移到STOP |
| SYS_STATE_EJECTING | 退碟中状态 | OLED第二行显示EJECTING，3s后迁移到NO DISC |
| SYS_STATE_STOP | 有碟停止状态 | OLED第二行显示STOP，第三行显示Music ### |
| SYS_STATE_PLAY | 播放状态 | OLED第二行显示PLAY，支持上一曲/下一曲 |
| SYS_STATE_PAUSE | 暂停状态 | OLED第二行显示PAUSE，长按上一曲/下一曲时连续切歌直到按键释放 |

### 2.2 状态流转约束

* 掉电约束：`SYS_STATE_POWER_OFF`下仅Power On事件有效，其余CD控制事件无效

* 无碟约束：`SYS_STATE_NO_DISC`下Play/Pause、Previous、Next无动作，仅Load/Eject有效

* 装退碟约束：`SYS_STATE_LOADING`和`SYS_STATE_EJECTING`为内部时序状态，3s完成后自动迁移

* 播放约束：有碟时`STOP -> PLAY`，`PLAY -> PAUSE`，`PAUSE -> PLAY`

* 切歌约束：共100首歌曲，上一曲到达首曲后回到第100首，下一曲到达第100首后回到第1首

* 掉电保存：Power Off时保存CD Last状态，Power On时恢复到上次Power On时的CD Last状态

## 三、线程（任务）划分与职责

### 3.1 线程分类与核心职责

| 线程名称 | 所属分组 | 核心职责 | 交互对象 |
| -------- | -------- | -------- | -------- |
| KEY线程 | G2 | 1. 扫描WK_UP、KEY_0、KEY_1；2. 实现30ms消抖、1.7s长按识别、按键抬起识别；3. 封装Key Message并发送 | Power线程、CD线程 |
| Power线程 | G4 | 1. 管理系统Power On/Off；2. 接收Power Key消息；3. 向CD/OLED发送系统电源状态通知；4. 统一喂独立看门狗 | KEY线程、CD线程、OLED线程、WatchDog |
| CD线程 | G3 | 1. 创建CD Message Queue；2. 维护CD状态机和歌曲编号；3. 处理Load/Eject、Play/Pause、Previous、Next事件；4. 向OLED发送CD状态和歌曲信息 | KEY线程、Power线程、OLED线程 |
| OLED线程 | G1 | 1. 接收Power/CD显示消息；2. 第一行显示Power状态；3. 第二行显示CD状态；4. 第三行显示Music ### | Power线程、CD线程、OLED Driver |
| OLED IIC驱动 | G1 | 1. 配置PB6/PB7 IIC通信；2. 实现OLED Driver；3. 支持全屏亮灭、字符显示、位图显示 | OLED线程 |
| WatchDog驱动 | G4 | 1. 初始化IWDG；2. 提供喂狗接口；3. 异常时辅助系统回归Power Off安全状态 | Power线程 |

### 3.2 线程约束规范

* 禁止线程之间直接调用对方业务处理函数或修改对方私有数据

* 模块内部允许维护`static`私有状态，例如CD当前状态、当前歌曲编号、是否有碟

* 跨线程业务通信只能通过消息队列传递结构体

* Power线程负责统一喂狗，禁止多个线程分散喂狗

* 硬件驱动中禁止调用RTOS延时、任务切换接口，仅允许使用BSP阻塞毫秒延时

## 四、状态机实现规划

### 4.1 实现方式：函数指针数组

* 设计逻辑：CD模块为每个“当前状态 + 输入事件”配置一个动作函数，通过二维函数指针数组查表执行

* 核心优势：状态迁移和动作处理分离，新增事件或状态时只需要同步扩展枚举、矩阵和动作函数

* 结构规划：

1. 定义状态枚举`SysState`

2. 定义事件枚举`MatrixEvent`

3. 定义动作函数指针类型`CdStateAction`

4. 构建状态迁移矩阵`g_stateMatrix[SYS_STATE_COUNT][MATRIX_EVENT_COUNT]`

5. 构建动作函数矩阵`s_actionMatrix[SYS_STATE_COUNT][MATRIX_EVENT_COUNT]`

6. CD线程收到消息后调用统一派发函数，根据当前状态和事件查表完成迁移

### 4.2 状态迁移：矩阵（Matrix）模型

#### 4.2.1 事件定义

| 事件名称 | 事件描述 | 触发条件 |
| -------- | -------- | -------- |
| MATRIX_EVENT_NONE | 无事件/无效事件 | 无按键操作或当前状态不响应该操作 |
| MATRIX_EVENT_POWER_ON | 系统上电事件 | WK_UP短按，Power模块确认系统Power On |
| MATRIX_EVENT_POWER_OFF | 系统掉电事件 | WK_UP长按1.7s以上，Power模块确认系统Power Off |
| MATRIX_EVENT_LOAD_EJECT | 装碟/退碟事件 | KEY_0短按 |
| MATRIX_EVENT_DISC_ACTION_DONE | 装碟/退碟完成事件 | CD内部Load/Eject动作持续3s后触发 |
| MATRIX_EVENT_PLAY_PAUSE | 播放/暂停事件 | KEY_1短按 |
| MATRIX_EVENT_PREVIOUS | 上一曲事件 | KEY_0长按1.7s以上 |
| MATRIX_EVENT_NEXT | 下一曲事件 | KEY_1长按1.7s以上 |
| MATRIX_EVENT_KEY_RELEASE | 按键抬起事件 | KEY_0或KEY_1长按后释放，用于结束连续切歌 |

#### 4.2.2 状态迁移矩阵

| 当前状态 \ 事件 | MATRIX_EVENT_POWER_ON | MATRIX_EVENT_POWER_OFF | MATRIX_EVENT_LOAD_EJECT | MATRIX_EVENT_DISC_ACTION_DONE | MATRIX_EVENT_PLAY_PAUSE | MATRIX_EVENT_PREVIOUS | MATRIX_EVENT_NEXT | MATRIX_EVENT_KEY_RELEASE | 其他事件 |
| --------------- | --------------------- | ---------------------- | ----------------------- | ----------------------------- | ----------------------- | --------------------- | ----------------- | ------------------------ | -------- |
| SYS_STATE_POWER_OFF | CD_LAST_STATE | - | - | - | - | - | - | - | MATRIX_EVENT_NONE |
| SYS_STATE_NO_DISC | - | SYS_STATE_POWER_OFF | SYS_STATE_LOADING | - | - | - | - | - | MATRIX_EVENT_NONE |
| SYS_STATE_LOADING | - | SYS_STATE_POWER_OFF | - | SYS_STATE_STOP | - | - | - | - | MATRIX_EVENT_NONE |
| SYS_STATE_EJECTING | - | SYS_STATE_POWER_OFF | - | SYS_STATE_NO_DISC | - | - | - | - | MATRIX_EVENT_NONE |
| SYS_STATE_STOP | - | SYS_STATE_POWER_OFF | SYS_STATE_EJECTING | - | SYS_STATE_PLAY | - | - | - | MATRIX_EVENT_NONE |
| SYS_STATE_PLAY | - | SYS_STATE_POWER_OFF | SYS_STATE_EJECTING | - | SYS_STATE_PAUSE | SYS_STATE_PLAY | SYS_STATE_PLAY | - | MATRIX_EVENT_NONE |
| SYS_STATE_PAUSE | - | SYS_STATE_POWER_OFF | SYS_STATE_EJECTING | - | SYS_STATE_PLAY | SYS_STATE_PAUSE | SYS_STATE_PAUSE | SYS_STATE_PAUSE | MATRIX_EVENT_NONE |

说明：

* `CD_LAST_STATE`表示恢复Power Off前保存的CD状态，代码中可先在矩阵填写默认状态，再由`app_cd_action_power_on`修正为历史状态

* `-`表示状态保持不变，且不执行有效动作

* `MATRIX_EVENT_PREVIOUS`和`MATRIX_EVENT_NEXT`在`SYS_STATE_PAUSE`下保持暂停状态，但按0.5s/首持续更新歌曲编号，直到`MATRIX_EVENT_KEY_RELEASE`

#### 4.2.3 状态迁移矩阵代码框架

```c
/* Event column order:
 * POWER_ON, POWER_OFF, LOAD_EJECT, DISC_ACTION_DONE,
 * PLAY_PAUSE, PREVIOUS, NEXT, KEY_RELEASE
 */
const SysState g_stateMatrix[SYS_STATE_COUNT][MATRIX_EVENT_COUNT] =
{
    /* POWER_OFF */
    {
        SYS_STATE_STOP,      SYS_STATE_POWER_OFF, SYS_STATE_POWER_OFF, SYS_STATE_POWER_OFF,
        SYS_STATE_POWER_OFF, SYS_STATE_POWER_OFF, SYS_STATE_POWER_OFF, SYS_STATE_POWER_OFF
    },
    /* NO_DISC */
    {
        SYS_STATE_NO_DISC,   SYS_STATE_POWER_OFF, SYS_STATE_LOADING,   SYS_STATE_NO_DISC,
        SYS_STATE_NO_DISC,   SYS_STATE_NO_DISC,   SYS_STATE_NO_DISC,   SYS_STATE_NO_DISC
    },
    /* LOADING */
    {
        SYS_STATE_LOADING,   SYS_STATE_POWER_OFF, SYS_STATE_LOADING,   SYS_STATE_STOP,
        SYS_STATE_LOADING,   SYS_STATE_LOADING,   SYS_STATE_LOADING,   SYS_STATE_LOADING
    },
    /* EJECTING */
    {
        SYS_STATE_EJECTING,  SYS_STATE_POWER_OFF, SYS_STATE_EJECTING,  SYS_STATE_NO_DISC,
        SYS_STATE_EJECTING,  SYS_STATE_EJECTING,  SYS_STATE_EJECTING,  SYS_STATE_EJECTING
    },
    /* STOP */
    {
        SYS_STATE_STOP,      SYS_STATE_POWER_OFF, SYS_STATE_EJECTING,  SYS_STATE_STOP,
        SYS_STATE_PLAY,      SYS_STATE_STOP,      SYS_STATE_STOP,      SYS_STATE_STOP
    },
    /* PLAY */
    {
        SYS_STATE_PLAY,      SYS_STATE_POWER_OFF, SYS_STATE_EJECTING,  SYS_STATE_PLAY,
        SYS_STATE_PAUSE,     SYS_STATE_PLAY,      SYS_STATE_PLAY,      SYS_STATE_PLAY
    },
    /* PAUSE */
    {
        SYS_STATE_PAUSE,     SYS_STATE_POWER_OFF, SYS_STATE_EJECTING,  SYS_STATE_PAUSE,
        SYS_STATE_PLAY,      SYS_STATE_PAUSE,     SYS_STATE_PAUSE,     SYS_STATE_PAUSE
    }
};
```

#### 4.2.4 动作函数矩阵

| 当前状态 \ 事件 | POWER_ON | POWER_OFF | LOAD_EJECT | DISC_ACTION_DONE | PLAY_PAUSE | PREVIOUS | NEXT | KEY_RELEASE |
| --------------- | -------- | --------- | ---------- | ---------------- | ---------- | -------- | ---- | ----------- |
| POWER_OFF | app_cd_action_power_on | - | - | - | - | - | - | - |
| NO_DISC | - | app_cd_action_power_off | app_cd_action_load | - | - | - | - | - |
| LOADING | - | app_cd_action_power_off | - | app_cd_action_load_done | - | - | - | - |
| EJECTING | - | app_cd_action_power_off | - | app_cd_action_eject_done | - | - | - | - |
| STOP | - | app_cd_action_power_off | app_cd_action_eject | - | app_cd_action_play | - | - | - |
| PLAY | - | app_cd_action_power_off | app_cd_action_eject | - | app_cd_action_pause | app_cd_action_previous | app_cd_action_next | - |
| PAUSE | - | app_cd_action_power_off | app_cd_action_eject | - | app_cd_action_play | app_cd_action_previous | app_cd_action_next | app_cd_action_key_release |

## 五、线程通信设计

### 5.1 通信方式

* 通信载体：消息队列（Message Queue / Mail Queue），采用结构体封装消息类型、来源模块、目标模块和扩展参数

* 消息格式：

```text
统一消息结构体（AppMsg）：

- srcModule：消息来源模块，如MODULE_KEY、MODULE_POWER、MODULE_CD
- dstModule：消息目标模块，如MODULE_POWER、MODULE_CD、MODULE_OLED
- msgId：消息类型，如Power状态、CD事件、CD状态、OLED刷新
- opt0：扩展参数0，8bit
- opt1：扩展参数1，8bit
- opt2：扩展参数2，8bit
- value：扩展值，如歌曲编号、状态值、计时值
```

* 通信方向：

1. KEY线程 -> Power线程：上报WK_UP短按/长按事件

2. KEY线程 -> CD线程：上报KEY_0、KEY_1短按/长按/释放事件

3. Power线程 -> CD线程：通知系统Power On/Power Off

4. Power线程 -> OLED线程：通知第一行Power显示内容

5. CD线程 -> OLED线程：通知第二行CD状态和第三行歌曲编号

### 5.2 线程交互流程

#### 5.2.1 基础上下电流程

1. KEY线程检测到WK_UP短按，满足30ms消抖条件后封装Power Key短按消息

2. Power线程接收Power Key消息，根据当前Power状态切换为Power On

3. Power线程向CD线程发送`MATRIX_EVENT_POWER_ON`，并向OLED线程发送Power On显示消息

4. CD线程查状态迁移矩阵，从`SYS_STATE_POWER_OFF`恢复到CD Last状态，默认可进入`SYS_STATE_STOP`

5. CD线程向OLED线程发送CD状态和Music ###显示消息

6. KEY线程检测到WK_UP长按1.7s以上，Power线程切换为Power Off

7. Power线程通知CD线程进入`SYS_STATE_POWER_OFF`，CD线程保存CD Last状态，并通知OLED刷新Power Off

#### 5.2.2 装碟/退碟流程

1. KEY线程检测到KEY_0短按，向CD线程发送`MATRIX_EVENT_LOAD_EJECT`

2. CD线程根据当前是否有碟查表迁移：`NO_DISC -> LOADING`或`STOP/PLAY/PAUSE -> EJECTING`

3. CD线程通知OLED第二行显示`LOADING`或`EJECTING`

4. CD内部启动3s模拟动作计时

5. 3s结束后CD线程触发`MATRIX_EVENT_DISC_ACTION_DONE`

6. `LOADING -> STOP`，OLED显示`STOP`和`Music 001`；`EJECTING -> NO_DISC`，OLED显示`NO DISC`

#### 5.2.3 播放/暂停流程

1. KEY线程检测到KEY_1短按，向CD线程发送`MATRIX_EVENT_PLAY_PAUSE`

2. CD线程根据状态迁移矩阵处理：`STOP -> PLAY`，`PLAY -> PAUSE`，`PAUSE -> PLAY`

3. 如果当前为`NO_DISC`，事件无效，不改变CD状态

4. CD线程向OLED线程发送最新CD状态和歌曲编号

#### 5.2.4 上一曲/下一曲流程

1. KEY线程检测到KEY_0长按1.7s以上，向CD线程发送`MATRIX_EVENT_PREVIOUS`

2. KEY线程检测到KEY_1长按1.7s以上，向CD线程发送`MATRIX_EVENT_NEXT`

3. `PLAY`状态下每次事件切换一首歌，状态保持`SYS_STATE_PLAY`

4. `PAUSE`状态下以0.5s/首连续切歌，直到KEY线程上报`MATRIX_EVENT_KEY_RELEASE`

5. 歌曲编号范围为1-100，上一曲到1后回到100，下一曲到100后回到1

## 六、功能模块详细规范

### 6.1 KEY按键响应模块

* 检测对象：WK_UP、KEY_0、KEY_1

* 消抖规则：30ms内连续3次检测到有效电平，判定按键按下

* 长按规则：连续1.7s检测到有效电平，判定长按

* 抬起规则：检测到一次无效电平，判定按键释放

* 映射关系：

| 按键 | 操作 | 事件 |
| ---- | ---- | ---- |
| WK_UP | 短按 | MATRIX_EVENT_POWER_ON |
| WK_UP | 长按1.7s以上 | MATRIX_EVENT_POWER_OFF |
| KEY_0 | 短按 | MATRIX_EVENT_LOAD_EJECT |
| KEY_0 | 长按1.7s以上 | MATRIX_EVENT_PREVIOUS |
| KEY_1 | 短按 | MATRIX_EVENT_PLAY_PAUSE |
| KEY_1 | 长按1.7s以上 | MATRIX_EVENT_NEXT |
| KEY_0 / KEY_1 | 长按释放 | MATRIX_EVENT_KEY_RELEASE |

### 6.2 Power电源管理模块

* Power Key触发后，根据系统Power状态执行相反动作：Power Off -> Power On，Power On -> Power Off

* Power On时向CD模块和OLED模块发送系统状态通知

* Power Off时要求CD模块停止响应其他按键，并保存CD Last状态

* 异常时系统进入Power Off安全状态，必要时触发看门狗复位

* 独立看门狗喂狗逻辑统一放在Power线程循环中

### 6.3 CD模拟播放器模块

* 必须创建CD线程和CD Message Queue

* 必须定义CD Event/Msg数据结构

* 必须使用`g_stateMatrix[SYS_STATE_COUNT][MATRIX_EVENT_COUNT]`实现状态迁移

* 必须使用函数指针数组实现动作处理

* 必须实现100首歌曲循环切换

* 必须实现Load/Eject 3s模拟动作和0.5s/首连续切歌动作

### 6.4 OLED显示控制模块

* 第一行显示Power状态：

  * `Power On`

  * `Power Off`

  * `CD Source`

* 第二行显示CD状态：

  * `NO DISC`

  * `LOADING`

  * `EJECTING`

  * `Fast Previousing`

  * `Fast Nexting`

  * `PLAY`

  * `PAUSE`

  * `STOP`

* 第三行显示歌曲编号：

  * 格式固定为`Music ###`

  * `###`为3位10进制数字，例如`Music 001`、`Music 100`

### 6.5 LED增值功能模块

* LED0连接PA8，LED1连接PD2

* LED0：Power On时，按键按下快速闪烁，按键抬起变为呼吸灯

* LED1：系统Power On时快速闪烁，Power Off时呼吸灯

## 七、文档维护与扩展说明

### 7.1 维护规范

* 新增状态：需同步更新状态枚举、状态迁移矩阵、动作函数矩阵、OLED显示映射

* 新增事件：需同步修改Key映射、CD事件枚举、状态迁移矩阵和动作函数矩阵

* 新增线程：需明确职责边界、消息格式、交互对象和队列长度

* 修改RTOS：本项目固定使用CMSIS-RTOS2 / RTX5，业务层与Core直接调用`osXxx()`原生API，不再保留`system_rtos`适配层；如需替换RTOS内核，统一在使用`osXxx()`的各文件按CMSIS-RTOS2标准接口调整

### 7.2 扩展建议

* 可扩展更多CD状态，如快进、快退、错误处理、碟片读取失败

* 可扩展OLED显示内容，如当前模式、错误码、调试信息

* 可扩展LED提示策略，用于区分Power、Key、CD动作和系统异常

* 可扩展单体测试和结合测试用例，覆盖状态矩阵中的每一条有效迁移
