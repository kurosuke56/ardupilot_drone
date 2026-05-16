"""
Simple indoor autonomous flight example using PyMAVLink.

Features:
- UDP connection to ArduPilot / ArduCopter
- Compass / EKF status monitoring
- Local NED position control
- Relative coordinate handling using arm position as origin
- Takeoff, waypoint flight, and landing

Important:
- This script assumes LOCAL_POSITION_NED is available.
- Intended for indoor flight with optical flow / rangefinder environments.
"""

from pymavlink import mavutil
import time
import math


# -----------------------------------------------------------------------------
# Configuration
# -----------------------------------------------------------------------------

UDP_PORT = 14551

# Allowed altitude error during takeoff completion check [m]
TAKEOFF_RANGE = 0.1

# Allowed XY position error for waypoint arrival [m]
ARRIVAL_RANGE = 0.1


# -----------------------------------------------------------------------------
# Global Position Cache
# -----------------------------------------------------------------------------

latest_position = {
    "x": 0,
    "y": 0,
    "z": 0
}


# -----------------------------------------------------------------------------
# Position Utilities
# -----------------------------------------------------------------------------

def update_position():
    """
    Receive and cache the latest LOCAL_POSITION_NED message.
    """

    msg = master.recv_match(
        type='LOCAL_POSITION_NED',
        blocking=True,
        timeout=1
    )

    if msg:
        latest_position["x"] = msg.x
        latest_position["y"] = msg.y
        latest_position["z"] = msg.z


def get_position():
    """
    Get the latest LOCAL_POSITION_NED position.

    Returns:
        tuple:
            (x, y, z) in LOCAL_NED coordinates
    """

    update_position()

    return (
        latest_position["x"],
        latest_position["y"],
        latest_position["z"]
    )


def get_local_position():
    """
    Convert global LOCAL_NED coordinates into relative coordinates
    using the arm position as the local origin.

    Returns:
        tuple:
            (local_x, local_y, local_z)
    """

    x, y, z = get_position()

    local_x = x - origin_x
    local_y = y - origin_y
    local_z = z - origin_z

    return local_x, local_y, local_z


# -----------------------------------------------------------------------------
# Flight Health Checks
# -----------------------------------------------------------------------------

def check_compass(timeout=10):
    """
    Monitor STATUSTEXT messages and check for compass / EKF issues.

    Returns:
        bool:
            True if no issues are detected within timeout.
            False if an error keyword is detected.
    """

    print("COMPASS / EKF CHECK")

    ng_words = [
        "compass variance",
        "ekf variance",
        "yaw inconsistent",
        "compass inconsistent",
        "ahrs"
    ]

    start = time.time()

    while time.time() - start < timeout:

        found_error = False

        while True:

            msg = master.recv_match(
                type='STATUSTEXT',
                blocking=False
            )

            if msg is None:
                break

            text = msg.text.lower()

            print("status:", text)

            for w in ng_words:

                if w in text:
                    print("NG:", w)
                    found_error = True

        if found_error:
            return False

        time.sleep(0.1)

    print("COMPASS OK")
    return True


def check_ekf_error():
    """
    Check STATUSTEXT messages for EKF / compass related errors.

    Returns:
        bool:
            True if an error is detected.
    """

    while True:

        msg = master.recv_match(
            type='STATUSTEXT',
            blocking=False
        )

        if msg is None:
            break

        text = msg.text.lower()

        ng_words = [
            "compass variance",
            "ekf variance",
            "yaw inconsistent",
            "prearm"
        ]

        for w in ng_words:

            if w in text:
                print("EKF ERROR:", text)
                return True

    return False


# -----------------------------------------------------------------------------
# Flight Commands
# -----------------------------------------------------------------------------

def set_guided():
    """
    Switch vehicle mode to GUIDED.
    """

    master.set_mode_apm('GUIDED')


def arm():
    """
    Arm the motors and wait until armed.
    """

    master.arducopter_arm()
    master.motors_armed_wait()


def disarm():
    """
    Disarm the motors and wait until disarmed.
    """

    master.arducopter_disarm()
    master.motors_disarmed_wait()


def takeoff(height):
    """
    Send takeoff command.

    Args:
        height (float):
            Target altitude in meters.
    """

    master.mav.command_long_send(
        master.target_system,
        master.target_component,
        mavutil.mavlink.MAV_CMD_NAV_TAKEOFF,
        0,
        0, 0, 0, 0,
        0, 0, height
    )


def wait_takeoff(target_height, timeout=15):
    """
    Wait until the vehicle reaches the target altitude.

    Returns:
        bool:
            True if takeoff completed successfully.
    """

    print("wait takeoff")

    start = time.time()

    while time.time() - start < timeout:

        if check_ekf_error():
            return False

        x, y, z = get_local_position()

        print(f"x={x:.2f} y={y:.2f} z={z:.2f}")

        # NED coordinate system:
        # Upward altitude becomes negative Z.
        if abs(abs(z) - abs(target_height)) <= TAKEOFF_RANGE:
            print("takeoff completed")
            return True

        time.sleep(0.2)

    return False


def land():
    """
    Send landing command.
    """

    master.mav.command_long_send(
        master.target_system,
        master.target_component,
        mavutil.mavlink.MAV_CMD_NAV_LAND,
        0,
        0, 0, 0, 0, 0, 0, 0
    )


def move(x, y, z):
    """
    Send LOCAL_NED position target.

    Args:
        x, y, z (float):
            Relative target position from local origin.
    """

    master.mav.set_position_target_local_ned_send(
        0,
        master.target_system,
        master.target_component,
        mavutil.mavlink.MAV_FRAME_LOCAL_NED,
        0b0000111111111000,
        x + origin_x,
        y + origin_y,
        z + origin_z,
        0, 0, 0,
        0, 0, 0,
        0,
        0
    )


def wait_arrival(tx, ty, tz, timeout=20):
    """
    Continue sending target position until the vehicle reaches it.

    Includes:
    - EKF monitoring
    - Position stagnation detection
    - Timeout detection

    Returns:
        bool:
            True if arrival succeeded.
    """

    print("wait arrival")

    start = time.time()

    last_dist = None
    stagnant_count = 0

    while time.time() - start < timeout:

        if check_ekf_error():
            return False

        # Re-send target continuously
        move(tx, ty, tz)

        x, y, z = get_local_position()

        dx = tx - x
        dy = ty - y

        dist = math.sqrt(dx * dx + dy * dy)

        print(f"x={x:.2f} y={y:.2f} z={z:.2f} dist={dist:.2f}")

        if dist < ARRIVAL_RANGE:
            print("arrival")
            return True

        # Detect stagnation (vehicle not moving)
        if last_dist is not None:

            if abs(last_dist - dist) < 0.05:
                stagnant_count += 1
            else:
                stagnant_count = 0

        if stagnant_count > 30:
            print("WARNING: position stagnant")
            return False

        last_dist = dist

        time.sleep(0.2)

    print("WARNING: arrival timeout")
    return False


# -----------------------------------------------------------------------------
# Main Sequence
# -----------------------------------------------------------------------------

try:
    input("Press Enter")

    # -------------------------------------------------------------------------
    # Initialize MAVLink connection
    # -------------------------------------------------------------------------

    print("bind udp")

    master = mavutil.mavlink_connection(
        f'udp:0.0.0.0:{UDP_PORT}'
    )

    print("wait heart beat")
    master.wait_heartbeat()

    # -------------------------------------------------------------------------
    # Pre-flight checks
    # -------------------------------------------------------------------------

    print("Compass Check")

    if not check_compass():
        raise Exception("compass check failed")

    # -------------------------------------------------------------------------
    # Switch to GUIDED mode
    # -------------------------------------------------------------------------

    print("guide")

    set_guided()
    time.sleep(1)

    # -------------------------------------------------------------------------
    # Arm
    # -------------------------------------------------------------------------

    print("arming")

    arm()
    time.sleep(2)

    # -------------------------------------------------------------------------
    # Save current position as local origin
    # -------------------------------------------------------------------------

    print("getting origin")

    origin_x, origin_y, origin_z = get_position()

    print(
        f"origin = "
        f"{origin_x:.2f}, "
        f"{origin_y:.2f}, "
        f"{origin_z:.2f}"
    )

    # -------------------------------------------------------------------------
    # Takeoff
    # -------------------------------------------------------------------------

    print("take off")

    takeoff(0.5)

    if not wait_takeoff(0.5):
        raise Exception("takeoff failed")

    # -------------------------------------------------------------------------
    # Waypoints
    # Coordinates are relative to arm position
    # -------------------------------------------------------------------------

    points = [
        [0, -0.5, -0.5],
        [0.5, -0.5, -0.5],
        [0.5,  0, -0.5],
        [0,  0, -0.5]
    ]
    #points = [
    #    [0, -0, -0.5],
    #    [1, -1, -0.5],
    #    [1,  0, -0.5],
    #    [0,  0, -0.5]
    #]


    # -------------------------------------------------------------------------
    # Move through each waypoint
    # -------------------------------------------------------------------------

    for p in points:

        print("move", p)

        if not wait_arrival(p[0], p[1], p[2]):
            raise Exception("arrival failed")

        time.sleep(0.5)

except Exception as e:

    print(f"Flight error: {e}")


# -----------------------------------------------------------------------------
# Landing Sequence
# -----------------------------------------------------------------------------

try:
    print("land")

    land()

except Exception as e:

    print(f"Landing error: {e}")


# -----------------------------------------------------------------------------
# Disarm
# -----------------------------------------------------------------------------

print("dis arm")

time.sleep(5)

disarm()

input("Press Enter to exit")