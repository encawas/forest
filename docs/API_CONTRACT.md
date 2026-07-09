# EchoForest API Contract

本文档定义网页、后端、S3、C5 之间的最小统一协议。

核心原则：后端是唯一状态中心。网页、S3、C5 都只能读取或请求后端状态，不能各自维护最终状态。

## 1. Canonical State

所有端统一读取同一个状态对象：

```json
{
  "version": 1,
  "mode": "focus",
  "forest_state": "growing",
  "reason": "环境正常，森林成长中",
  "environment": {
    "light": 300,
    "noise": 40,
    "temp": 24,
    "humi": 50,
    "updatedAt": "2026-07-09T08:01:00.000Z",
    "source": "s3"
  },
  "session": {
    "id": "session-1783584000000",
    "goal": "finish probability review",
    "active": true,
    "startedAt": "2026-07-09T08:00:00.000Z",
    "endedAt": null,
    "events": [],
    "warnings": {
      "light_warning": 0,
      "noise_warning": 0,
      "comfort_warning": 0
    },
    "environmentSummary": {}
  },
  "devices": {
    "s3-main": {
      "id": "s3-main",
      "role": "s3",
      "source": "s3",
      "online": true,
      "lastSeenAt": "2026-07-09T08:01:00.000Z"
    }
  },
  "logs": [],
  "updatedAt": "2026-07-09T08:01:00.000Z"
}
```

## 2. Mode

`mode` 表示学习流程模式：

| mode | 含义 |
|---|---|
| `idle` | 等待开始 |
| `focus` | 学习中 |
| `break` | 休息中 |
| `completed` | 学习完成 |

## 3. Forest State

`forest_state` 表示森林反馈状态：

| forest_state | 含义 |
|---|---|
| `idle` | 等待开始 |
| `growing` | 环境正常，森林成长中 |
| `light_warning` | 光线过暗 |
| `noise_warning` | 噪声偏高 |
| `comfort_warning` | 温湿度不适 |
| `break` | 休息中 |
| `completed` | 学习完成 |

## 4. Environment Data

S3 上传真实环境数据：

```json
{
  "light": 300,
  "noise": 40,
  "temp": 24,
  "humi": 50,
  "source": "s3",
  "deviceId": "s3-main"
}
```

字段要求：

| 字段 | 含义 |
|---|---|
| `light` | 光照数据，数值型 |
| `noise` | 噪声数据，数值型 |
| `temp` | 温度数据，数值型 |
| `humi` | 湿度数据，数值型 |
| `source` | 数据来源，默认 `s3` |
| `deviceId` | 设备 ID，默认 `s3-main` |

当前后端只要求四个环境字段是有限数值。真实单位和阈值应在后续硬件校准时固定。

## 5. HTTP Endpoints

### 5.1 Health Check

```http
GET /health
```

返回：

```json
{
  "ok": true,
  "service": "echoforest-backend"
}
```

### 5.2 Get State

网页、S3、C5 都读取这个接口：

```http
GET /api/state
```

返回 canonical state。

### 5.3 Start Focus

网页或 C5 触发开始学习：

```http
POST /api/control/start_focus
Content-Type: application/json

{
  "goal": "finish probability review",
  "source": "web"
}
```

`source` 可为：

- `web`
- `c5`
- `simulator`

返回：

```json
{
  "ok": true,
  "state": {}
}
```

### 5.4 Start Break

网页或 C5 触发休息：

```http
POST /api/control/start_break
Content-Type: application/json

{
  "source": "c5"
}
```

返回最新状态。

### 5.5 End Focus

网页或 C5 触发结束学习：

```http
POST /api/control/end_focus
Content-Type: application/json

{
  "source": "web"
}
```

返回：

```json
{
  "ok": true,
  "state": {},
  "log": {
    "goal": "finish probability review",
    "durationSec": 1800,
    "summary": "本轮学习环境整体稳定，森林顺利成长。"
  }
}
```

### 5.6 S3 Upload Environment Data

```http
POST /api/device_data
Content-Type: application/json

{
  "light": 300,
  "noise": 40,
  "temp": 24,
  "humi": 50,
  "source": "s3",
  "deviceId": "s3-main"
}
```

返回最新状态。

### 5.7 Device Heartbeat

S3 和 C5 都可以发心跳：

```http
POST /api/devices/heartbeat
Content-Type: application/json

{
  "deviceId": "c5-control",
  "role": "c5",
  "source": "c5"
}
```

返回最新状态。

### 5.8 Get Logs

```http
GET /api/logs
```

返回：

```json
{
  "logs": []
}
```

## 6. Device Responsibilities

### Web

- 调用 `start_focus`、`start_break`、`end_focus`。
- 轮询或订阅 `/api/state`。
- 显示当前模式、森林状态、环境数据、设备在线状态和学习日志。

### S3

- 采集真实 `light/noise/temp/humi`。
- 调用 `/api/device_data` 上传环境数据。
- 调用 `/api/state` 获取后端判断后的森林状态。
- 主屏显示后端状态，不独立决定最终状态。

### C5

- 触摸后调用 `start_focus`、`start_break`、`end_focus`。
- 调用 `/api/state` 获取后端状态。
- 副屏显示 mode、forest_state、environment。
- 后续根据 forest_state 变化触发提示音或语音。

## 7. State Consistency Rules

1. 后端状态版本号 `version` 每次变化递增。
2. 设备显示时应记录自己看到的 `version`。
3. 如果控制请求失败，设备不能只更新本地 UI。
4. 如果 S3/C5 断线，后端后续应通过 `lastSeenAt` 判断在线状态。
5. 网页、S3、C5 都必须以 `/api/state` 返回值为准。
