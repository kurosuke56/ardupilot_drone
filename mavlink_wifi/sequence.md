sequenceDiagram
    autonumber

    participant PY as pymavlink
    participant MP as ミッションプランナー
    participant ESP as M5StampPico
    participant FC as フライトコントローラー

    %% 初期化
    ESP->>ESP: Wi-Fi初期化
    ESP->>ESP: UDPソケット開始

    %% ユーザー操作
    PY->>ESP: UDP接続の確立
    ESP->>ESP: GCS（PCの認識と送信先の登録）

    MP->>ESP: UDP接続の確立


    loop
        alt 接続断
            ESP->>ESP: 再接続処理
        end

        MP->>ESP: MAVLink (UDP)
        ESP->>FC: MAVLink (UART)

        PY->>ESP: MAVLink (UDP)
        ESP->>FC: MAVLink (UART)

        FC->>ESP: MAVLink (UART)
        ESP->>MP: MAVLink (UDP)
        ESP->>PY: MAVLink (UDP)
    end