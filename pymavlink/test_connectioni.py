from pymavlink import mavutil
import time

# ==========================================
# UDP待受ポート
# ==========================================

UDP_PORT = 14551

print(f"Listening UDP : {UDP_PORT}")

# ==========================================
# MAVLink接続
# ==========================================

master = mavutil.mavlink_connection(
    f'udp:0.0.0.0:{UDP_PORT}'
)

print("Waiting heartbeat...")

# Heartbeat待機
master.wait_heartbeat(timeout=10)

print("================================")
print("Heartbeat received")
print("================================")

print(f"System ID     : {master.target_system}")
print(f"Component ID  : {master.target_component}")

# ==========================================
# メッセージ受信確認
# ==========================================

print("\nReceiving MAVLink messages...\n")

while True:

    msg = master.recv_match(
        blocking=True,
        timeout=5
    )

    if msg is None:
        print("Timeout...")
        continue

    msg_type = msg.get_type()

    print(f"[{msg_type}]")

    # LOCAL_POSITION_NED表示
    if msg_type == "LOCAL_POSITION_NED":

        print(
            f"X={msg.x:.2f} "
            f"Y={msg.y:.2f} "
            f"Z={msg.z:.2f}"
        )

    # HEARTBEAT表示
    elif msg_type == "HEARTBEAT":

        print(
            f"Mode={master.flightmode}"
        )

    time.sleep(0.01)