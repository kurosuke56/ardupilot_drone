sequenceDiagram

    participant FC as フライトコントローラー
    participant ESP as M5StampPico
    participant MP as ミッションプランナー

    %% 初期化
    ESP->>ESP: Wi-Fi初期化
    ESP->>ESP: UDPソケット開始

    %% 接続確立
    MP->>ESP: UDP接続要求
    ESP-->>MP: 接続応答

    loop
        alt 接続断
            ESP->>ESP: 再接続処理
        end

        FC->>ESP: MAVLink (UART)
        ESP->>MP: MAVLink (UDP)


        MP->>ESP: MAVLink (UDP)
        ESP->>FC: MAVLink (UART)
    end