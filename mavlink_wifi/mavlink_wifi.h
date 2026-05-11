// Wifi接続設定
const int RETRY_COUNT_MAX = 10;
const int RETRY_DELAY_MSEC = 500;
const int RETRY_WARNING_FLASH_TIME = 200;

// ===== LED設定 =====
#define PIN 27
Adafruit_NeoPixel led(1, PIN, NEO_GRB + NEO_KHZ800);

// ===== MAVLink UDP =====
WiFiUDP udpMav;
const int MAV_PORT = 14550;     // 受信用
const int SEND_PORT_MP = 14550; // Mission Planner送信先
const int SEND_PORT_PY = 14551; // PYMAVLINK送信先

// ===== UART =====
#define RXD2 32
#define TXD2 33
#define MAVLINK_BUFFER 512
const int BAUD_RATE = 115200;

// ===== UART =====
HardwareSerial mavSerial(2);
uint8_t buffer[MAVLINK_BUFFER];

// ===== GCS情報 =====
IPAddress gcsIP;
//uint16_t gcsPort = 0;
bool gcsConnected = false;
