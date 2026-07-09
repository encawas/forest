# C5 Package Capability Supplement

本文档补充说明 `c5 code.zip` 中可以用于 EchoForest C5 端能力补齐的信息。

当前判断：`c5 code.zip` 不应被当成最终主工程直接替代 `firmware_modules/c5`，而应作为 C5 原始工程/硬件能力参考包，用来补齐 C5 端显示、联网、音频提示、SPI 总线和 ESP-IDF 工程配置等能力。

## 1. C5 在 EchoForest 中的最终职责

C5 的最终定位是：

- 触摸控制终端：支持开始学习、进入休息、结束学习。
- 状态副屏：显示当前模式、森林状态、环境数据和后端反馈。
- 语音提示设备：后续在关键状态变化时进行语音提醒或提示音提醒。
- 后端状态客户端：所有显示和控制都以后端状态为准，不维护独立最终状态。

C5 不能自己成为状态中心，也不能独立判断最终森林状态。C5 的触摸动作只负责向后端发出控制请求，最终状态必须以后端响应为准。

## 2. C5 包中已经能确认的能力

从 `c5 code.zip` 的文件结构可以确认，C5 包中包含以下能力模块。

### 2.1 LCD / 屏幕显示能力

相关文件：

```text
c5/components/LCD/animation.c
c5/components/LCD/animation.h
c5/components/LCD/lcd.c
c5/components/LCD/lcd.h
```

可用于补齐：

- C5 副屏基础显示能力。
- 森林状态或模式状态的屏幕渲染基础。
- 后续简单动画或状态切换动画。
- 与 `firmware_modules/c5/control_screen_renderer.c` 对接。

主工程中已有 `control_screen_renderer.c`，但它更像 EchoForest 业务层渲染入口；C5 包中的 LCD 组件更像底层屏幕驱动和动画能力。后续应把二者关系整理为：

```text
后端状态 -> C5 app -> control_screen_renderer -> LCD/lcd + LCD/animation
```

### 2.2 Wi-Fi 联网能力

相关文件：

```text
c5/components/wifi/wifi.c
c5/components/wifi/wifi.h
c5/main/softap_sta.c
```

可用于补齐：

- C5 连接现场 Wi-Fi 的基础能力。
- C5 作为后端客户端访问后端 API 的联网基础。
- 可能包含 softAP/STA 模式切换或配网相关参考逻辑。
- 与 `firmware_modules/common/wifi_manager.c` 对接。

主工程中已有 `firmware_modules/common/wifi_manager.c`，因此后续不是简单复制两个 Wi-Fi 实现，而是要明确一个最终 Wi-Fi 管理入口，避免 C5 一套、common 一套造成重复和冲突。

推荐关系：

```text
C5 原始 Wi-Fi 代码 -> 提取可用初始化/连接逻辑 -> 统一进入 common/wifi_manager
```

### 2.3 SPEAKER / 音频提示能力

相关文件：

```text
c5/components/SPEAKER/pwm.c
c5/components/SPEAKER/pwm.h
c5/components/SPEAKER/mic.c
c5/components/SPEAKER/mic.h
```

可用于补齐：

- C5 后续语音提醒或提示音提醒能力。
- 状态变化时的音频反馈。
- PWM 播放提示音或驱动扬声器的底层能力。
- MIC 相关代码可作为音频输入/硬件验证参考，但当前最终闭环中环境噪声采集主要应由 S3 承担。

需要注意：当前能确认的是 C5 包中有 SPEAKER/PWM/MIC 相关组件，但不能直接等价于已经具备完整 TTS 语音播报能力。最终语音提醒至少还需要：

- 后端状态变化事件。
- C5 侧状态变化检测。
- 状态到提示音/语音内容的映射。
- 防重复播报逻辑。
- 播放失败或设备离线处理。

推荐关系：

```text
后端 forest_state 变化 -> C5 拉取/接收新状态 -> speaker_event 判断 -> pwm/音频模块播放提示
```

### 2.4 SPI 总线能力

相关文件：

```text
c5/components/SPI/spi.c
c5/components/SPI/spi.h
```

可用于补齐：

- LCD 或其他外设的底层通信。
- 屏幕驱动初始化和数据传输。
- C5 硬件工程迁移时的总线配置参考。

该模块更偏硬件底层，不应承载 EchoForest 业务状态。

### 2.5 ESP-IDF 工程配置能力

相关文件：

```text
c5/CMakeLists.txt
c5/main/CMakeLists.txt
c5/main/Kconfig.projbuild
c5/main/idf_component.yml
c5/sdkconfig
c5/sdkconfig.defaults
c5/partitions.csv
c5/.vscode/
c5/.devcontainer/
```

可用于补齐：

- C5 工程的 ESP-IDF 编译配置。
- 组件依赖关系。
- 分区表。
- 开发容器和 VS Code 配置。
- Kconfig 配置项参考。

这些文件说明 C5 包更接近一个完整 ESP-IDF 工程，而 `firmware_modules/c5` 更像 EchoForest 主包里的业务模块骨架。

## 3. C5 包能补齐的 EchoForest 能力点

| EchoForest 目标能力 | C5 包可补充的信息 | 仍缺的 EchoForest 接入 |
|---|---|---|
| C5 副屏显示当前模式 | LCD/lcd、LCD/animation | 后端状态到屏幕 UI 的字段映射 |
| C5 显示森林状态 | LCD/animation | `forest_state` 到图形/文字/动画的映射 |
| C5 显示环境数据 | LCD 显示能力 | 从后端获取 S3 上传后的 `light/noise/temp/humi` |
| C5 触摸控制 | 主包已有 `touch_actions.c`，C5 包提供底层工程参考 | 触摸动作调用后端控制 API |
| C5 联网 | wifi.c、wifi.h、softap_sta.c | 与 `common/wifi_manager` 和 `cloud_client` 统一 |
| C5 语音提醒 | SPEAKER/pwm、SPEAKER/mic | 状态变化事件、播报映射、防重复播报 |
| C5 编译运行 | CMake、sdkconfig、partitions | 与主工程目录结构和组件依赖合并 |

## 4. 当前 C5 侧剩余缺口

即使考虑 `c5 code.zip`，C5 侧仍然缺以下闭环能力：

### 4.1 C5 不是后端状态客户端

需要补齐：

```text
C5 定时获取后端状态
C5 根据后端状态刷新屏幕
C5 触摸操作发送给后端
C5 等待后端确认后再更新显示
```

不能让 C5 本地点击后只改本地 UI。

### 4.2 C5 显示字段还需要统一

C5 副屏至少应显示：

```text
mode: focus / break / idle / completed
forest_state: growing / light_warning / noise_warning / comfort_warning / completed / break
light
noise
temp
humi
last_update
backend connection status
s3 online/offline
```

字段名需要和后端、网页、S3 使用同一套协议。

### 4.3 C5 触摸动作需要绑定后端控制命令

C5 触摸动作最终只能产生以下后端命令：

```text
start_focus
start_break
end_focus
```

触摸动作不能直接决定最终 `forest_state`，只能请求后端更新。

### 4.4 C5 语音提醒还只是可接入能力

C5 包中的 SPEAKER/PWM 说明它有音频输出基础，但最终语音提醒仍需补齐：

```text
previous_state
current_state
state_changed
prompt_type
last_prompt_time
```

并且要避免状态刷新时反复播报同一句提醒。

### 4.5 C5 Wi-Fi 代码需要和 common 层统一

主工程已有：

```text
firmware_modules/common/wifi_manager.c
firmware_modules/common/cloud_client.c
firmware_modules/common/echoforest_protocol.h
```

C5 包也有 Wi-Fi 代码。最终应避免两套 Wi-Fi 管理逻辑并存。建议把 C5 包中的可用硬件初始化和连接细节吸收到 common 层或 C5 app 初始化层。

## 5. C5 接入后的目标链路

C5 最终链路应是：

```text
用户触摸 C5
-> touch_actions 生成控制意图
-> cloud_client 向后端发送 start_focus/start_break/end_focus
-> 后端更新唯一状态
-> C5 拉取/接收后端最新状态
-> control_screen_renderer 渲染副屏
-> LCD 驱动显示模式、森林状态、环境数据
-> 如 forest_state 发生关键变化，SPEAKER 播放提示音/语音
```

C5 显示的环境数据必须来自后端状态，而不是 C5 自己凭空生成。环境数据源头仍然是 S3。

## 6. C5 与 S3 的边界

C5：

- 负责用户控制。
- 负责副屏显示。
- 负责语音/提示音反馈。
- 可以显示环境数据，但不作为最终环境数据源。

S3：

- 负责真实采集 `light/noise/temp/humi`。
- 负责上传环境数据到后端。
- 负责主森林屏显示。

后端：

- 负责唯一状态判断。
- 负责 session 和日志。
- 负责把同一个状态同步给网页、S3、C5。

## 7. C5 包补齐后的结论

加入 `c5 code.zip` 的信息后，当前项目判断应更新为：

- C5 并不是只有业务骨架；它已有较完整的 ESP-IDF 原始工程能力。
- C5 包可补齐 LCD 显示、Wi-Fi、SPEAKER/PWM、SPI、ESP-IDF 配置等底层信息。
- 但 C5 仍缺 EchoForest 协议接入、后端状态同步、触摸控制命令、语音提醒事件逻辑。
- C5 包最适合作为硬件适配参考，不应直接成为状态中心。

最终重点不是“让 C5 单独跑起来”，而是让 C5 成为 EchoForest 后端状态中心下的一个可靠客户端。
