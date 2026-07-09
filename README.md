# EchoForest

EchoForest 的最终目标是实现一个“网页 + 后端 + 双 ESP32 硬件”的完整学习环境反馈闭环。

一句话目标：EchoForest 最终要实现一个能真实采集学习环境、统一判断学习状态、同步驱动网页和双屏硬件反馈，并在学习结束后生成日志的 AIoT 学习森林系统。

## 最终系统定位

- 网页：演示和调试总控台。
- 后端：唯一状态中心。
- S3：环境感知设备和主森林屏。
- C5：触摸控制、副屏状态显示和后续语音提示设备。

所有端都必须以同一个后端状态为准，不能出现网页、S3、C5 各自维护状态导致显示不一致的情况。

## 当前已落地能力

当前仓库已经从“只有 zip 上传包”推进为有可继续开发的源码骨架：

- `backend/`：Node/Express 后端状态中心，提供统一 API、环境数据接收、状态判断、session 和学习日志。
- `frontend/`：Vite + React 网页控制台，支持开始学习、休息、结束学习、状态轮询、环境数据展示、设备状态和日志展示。
- `firmware_modules/common/`：固件通用协议和云端客户端接口，统一 `mode`、`forest_state`、环境数据和控制动作。
- `firmware_modules/c5/`：C5 业务接入层，包含触摸动作映射、副屏渲染、后端状态轮询和语音/提示音事件映射。
- `firmware_modules/s3/`：S3 业务接入层，包含 mock/real sensor 适配、环境上传和主屏状态渲染。
- `simulator/`：设备模拟器，可模拟网页/C5 控制、S3 环境数据上传、状态变化和学习日志生成。
- `docs/`：API 协议和 C5 包能力补充说明。

原始上传包仍保留：

- `echoforest-onsite-clean-20260709-160601.zip`
- `c5 code.zip`

其中 `c5 code.zip` 是 C5 原始工程/硬件能力补充包，包含 LCD、Wi-Fi、SPEAKER、SPI、main 等 ESP-IDF 组件。当前主工程已先接入 C5 业务层；真实 LCD/Wi-Fi/SPEAKER 驱动迁移仍需要实机引脚和 ESP-IDF 配置确认后再做。

## 快速运行

要求 Node.js >= 20。

### 1. 安装后端依赖

```bash
npm run backend:install
```

### 2. 启动后端

```bash
npm run backend:start
```

默认后端地址：

```text
http://127.0.0.1:3001
```

### 3. 启动前端

另开一个终端：

```bash
npm run frontend:install
npm run frontend:dev
```

默认前端地址：

```text
http://127.0.0.1:5173
```

### 4. 运行设备模拟器

后端启动后执行：

```bash
npm run simulator:demo
```

模拟器会模拟：

```text
C5/S3 心跳
网页开始学习
S3 上传 light/noise/temp/humi
后端判断 growing/light_warning/noise_warning/comfort_warning
C5 进入休息
C5 恢复学习
网页结束学习
后端生成日志
```

## 测试

后端状态引擎和 HTTP API 测试：

```bash
npm run backend:test
```

设备模拟器测试：

```bash
npm run simulator:test
```

前端构建检查：

```bash
npm run frontend:build
```

## 核心 API

详见：

- `docs/API_CONTRACT.md`

主要接口：

```text
GET  /api/state
GET  /api/logs
POST /api/control/start_focus
POST /api/control/start_break
POST /api/control/end_focus
POST /api/device_data
POST /api/devices/heartbeat
```

## 最终演示闭环

1. 用户通过网页或 C5 触摸屏点击“开始学习”。
2. 后端进入学习中状态。
3. S3 采集真实环境数据：`light`、`noise`、`temp`、`humi`。
4. 后端根据环境数据判断当前森林状态：
   - 环境正常：森林成长中
   - 光线过暗：光线提醒
   - 噪声偏高：噪声提醒
   - 温湿度不适：舒适度提醒
   - 学习结束：学习完成
   - 休息模式：休息中
5. 网页实时显示当前学习状态、环境数据、设备反馈和学习日志。
6. S3 主屏显示对应的森林状态、提示文字和后续动画效果。
7. C5 副屏显示当前模式、森林状态、环境数据，并支持触摸控制。
8. C5 后续可以在关键状态变化时进行语音提醒。
9. 用户点击“结束学习”后，后端生成学习日志，网页显示本轮目标、用时和总结。

## 仍需实机确认的部分

以下部分已经有业务接入层或适配层，但不能在没有硬件参数的情况下盲目完成：

1. C5 真实 LCD 驱动、SPI 引脚、屏幕初始化和动画资源接入。
2. C5 真实 Wi-Fi 初始化逻辑与 `common/wifi_manager` 的最终合并。
3. C5 SPEAKER/PWM 真实语音或提示音播放。
4. S3 真实 light/noise/temp/humi 传感器型号、引脚、采样单位和校准阈值。
5. ESP-IDF CMake、sdkconfig、partitions 与具体板卡工程的最终合并。

当前仓库已经具备“无实机模拟闭环”和“固件业务层接入骨架”；接下来接真实硬件时，应优先把 C5 包中的底层 LCD/Wi-Fi/SPEAKER/SPI 能力按板卡参数并入这些业务接口。
