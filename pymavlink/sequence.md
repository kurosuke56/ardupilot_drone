---
config:
  theme: default
  fontFamily: '''Source Code Pro Variable'', monospace'
---
sequenceDiagram
    autonumber
    participant PC
    participant ドローン（ArduPilot）


    PC->>PC: UDP接続待機開始
    PC->>ドローン（ArduPilot）: ハートビート待機
    PC->>ドローン（ArduPilot）: Compass・EKFの取得とチェック
    PC->>ドローン（ArduPilot）: GUIDEDモードに変更
    PC->>ドローン（ArduPilot）: ARM
    PC->>ドローン（ArduPilot）: 原点座標取得
    PC->>ドローン（ArduPilot）: 離陸(0.5m)

    loop 指定された座標に順番に移動
        PC->>ドローン（ArduPilot）: 指定座標に移動
        PC->>ドローン（ArduPilot）: 指定座標の到着確認

    end

    PC->>ドローン（ArduPilot）: 着陸
    PC->>ドローン（ArduPilot）: DISARM
