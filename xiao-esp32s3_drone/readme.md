# XIAO ESP32-S3 Drone FC beta

ESP32を利用した小型ドローン向けフライトコントローラーです。
スマートフォンのブラウザから操作し、WebSocketによるリアルタイム通信でモーター制御を行います。

現在はフライトコントローラーの基本機能（通信・Mixer・モーター制御・フェイルセーフ）を実装しています。

---

## Features

* XIAO ESP32-S3 ベース
* FreeRTOSによる制御タスク分離
* Webブラウザ操縦
* WebSocketリアルタイム通信
* 4モーターPWM制御
* Mixer計算
* FailSafe機能
* LittleFS対応
* テレメトリ送信

---

## System Architecture

```text
Smartphone Browser
        │
        │ WebSocket
        ▼
TelemetryServer
        │
        │ SharedData
        ▼
FlightController
        │
        │ PWM
        ▼
Motor Driver
        │
        ▼
Motors
```

---

## Software Structure

```text
DroneFC
│
├─ DroneFC.ino
│      メイン処理
│
├─ FlightController.h/.cpp
│      フライト制御
│      ・Mixer
│      ・PWM出力
│      ・FailSafe
│
├─ TelemetryServer.h/.cpp
│      通信管理
│      ・WiFi
│      ・HTTP
│      ・WebSocket
│
├─ SharedData.h
│      タスク間共有データ
│
└─ data/
       Web UI
       ・HTML
       ・CSS
       ・JavaScript
```

---

## Motor Layout

モーター配置

```text
        Front

    M4          M1

    M3          M2

        Rear
```

現在のMixer計算

```cpp
m1 = throttle-pitch-roll-yaw;

m2 = throttle+pitch-roll+yaw;

m3 = throttle+pitch+roll-yaw;

m4 = throttle-pitch+roll+yaw;
```

入力方向

| 入力       |   意味 |
| -------- | ---: |
| Throttle |   上下 |
| Pitch    | 前後傾き |
| Roll     | 左右傾き |
| Yaw      |   回転 |

---

## Flight Control Loop

制御周期:

```cpp
CONTROL_PERIOD_MS = 4
```

動作周波数:

```text
250Hz
```

フライト制御は FreeRTOS の独立タスクとして動作します。

```cpp
while(true){

    fc.update();

    vTaskDelayUntil(
        &lastWakeTime,
        pdMS_TO_TICKS(
            CONTROL_PERIOD_MS));
}
```

---

## FailSafe

通信が一定時間途絶えた場合、モーターを自動停止します。

```cpp
FAILSAFE_TIMEOUT_MS = 300
```

動作:

```text
通信断
    ↓
300ms経過
    ↓
ARM解除
    ↓
モーター停止
```

---

## Telemetry

ブラウザへ送信される情報:

```json
{
  "m1":120,
  "m2":118,
  "m3":123,
  "m4":122,
  "armed":true,
  "rssi":-52
}
```

内容:

| 項目    | 説明       |
| ----- | -------- |
| m1～m4 | モーター出力   |
| armed | ARM状態    |
| rssi  | WiFi電波強度 |

---

## Required Libraries

Arduino IDEで以下をインストールしてください。

* ESP32 Board Package
* WebServer
* WebSockets
* ArduinoJson
* LittleFS
* ESPmDNS

---

## Hardware

Controller:

* XIAO ESP32-S3

Motor Driver:

* TBD

IMU:

* TBD

Motor:

* Coreless DC Motor

Battery:

* 1S LiPo

---

## Current Status

Implemented:

* WiFi通信
* WebSocket
* モーターPWM
* Mixer
* FailSafe
* テレメトリ

Planned:

* IMU対応
* PID制御
* 姿勢推定
* キャリブレーション
* バッテリー監視
* 高度制御

---

## License

MIT License

---

## Author

くろすけ

```
```
