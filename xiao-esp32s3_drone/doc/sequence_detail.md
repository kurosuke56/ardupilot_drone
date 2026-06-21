---
config:
  layout: elk
  theme: neutral
  look: neo
---
sequenceDiagram
  autonumber

  participant B as ブラウザ（スマホ）
  participant W as ウェブサーバー
  participant F as フライトタスク
  participant M as モーター

Note over B: ジョイスティック操作開始

loop 20ms周期 (50Hz)
    B->>W: RC送信(thr,yaw,pit,rol,arm)
    activate W

    W->>W: JSON解析・RC更新
    deactivate W
end

loop 4ms周期 (250Hz)
    activate F

    W->>F: 最新RCデータ共有

    F->>F: フェイルセーフ確認

    alt ARM=true
        F->>F: Mixer演算<br/>M1〜M4計算

        F->>M: PWM出力(M1〜M4)
        activate M
        M-->>F: モーター回転状態
        deactivate M

    else ARM=false または通信断
        F->>M: PWM=0
    end

    F-->>W: Telemetry更新<br/>(M1,M2,M3,M4,RSSI)

    deactivate F
end

loop 50ms周期
    W-->>B: Telemetry送信
    B->>B: モータバー更新<br/>RSSI更新<br/>Latency表示
end