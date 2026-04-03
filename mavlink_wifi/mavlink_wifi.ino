#include <WiFi.h>
#include <WiFiUdp.h>
#include <HardwareSerial.h>
#include <Adafruit_NeoPixel.h>
#include "preference.h"
#include "mavlink_wifi.h"


// ===== YELLOW LED(Connecting) =====
void ShowLEDConnecting() {
  led.setPixelColor(0, led.Color(255, 223, 0));
  led.show();
}

// ===== RED LED(Warning) =====
void ShowLEDWarning() {
  led.setPixelColor(0, led.Color(255, 0, 0));
  led.show();
}

// ===== GREEN LED(Connected) =====
void ShowLEDConnected() {
  led.setPixelColor(0, led.Color(0, 255, 0));
  led.show();
}

// ===== Wi-Fi接続 =====
void ConnectWifi() {
  while (1)
  {
    int retry_count = 0;
    int connected = false;
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // 指定回数リトライ
    for (int retry_count = 0; retry_count < RETRY_COUNT_MAX; retry_count++)
    {
      if (WiFi.status() == WL_CONNECTED) {
        connected = true;
        break;
      }
        delay(RETRY_DELAY_MSEC);
    }

    if (connected)
    {
      break;
    }

    ShowLEDWarning();
    delay(RETRY_WARNING_FLASH_TIME);
    ShowLEDConnecting();
  }
}

// ===== setup =====
void setup() {
  Serial.begin(BAUD_RATE);
  led.begin();

  ShowLEDConnecting();

  mavSerial.begin(BAUD_RATE, SERIAL_8N1, RXD2, TXD2);

  // Wi-Fi接続
  ConnectWifi();

  // MAVLink開始
  udpMav.begin(MAV_PORT);

  ShowLEDConnected();
}

// ===== loop =====
void loop() {

  // WiFi切断時は再接続
  if (WiFi.status() != WL_CONNECTED) {
    ConnectWifi();
  }

  // ===== FC → PC =====
  int availableBytes = mavSerial.available();
  if (availableBytes > 0) {
    int len = mavSerial.readBytes(buffer, min(availableBytes, MAVLINK_BUFFER));

    udpMav.beginPacket(GROUND_STATION_IP, MAV_PORT);
    udpMav.write(buffer, len);
    udpMav.endPacket();
  }

  // ===== PC → FC =====
  int packetSize = udpMav.parsePacket();
  if (packetSize) {
    int len = udpMav.read(buffer, MAVLINK_BUFFER);
    mavSerial.write(buffer, len);
  }
}
