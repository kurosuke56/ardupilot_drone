#include <WiFi.h>
#include "M5Atom.h"

uint8_t LED_signal = 0;

void setup() {
  Serial.begin(115200);

  M5.begin(true, false, true);

  WiFi.softAP("ESP32_TEST", "12345678");

  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  switch (LED_signal)
  {
    case 0:
        M5.dis.drawpix(0, 0xFFFFFF);
        break;
    case 1:
        M5.dis.drawpix(0, 0xFF0000);
        break;
    case 2:
        M5.dis.drawpix(0, 0x00FF00);
        break;
    case 3:
        M5.dis.drawpix(0, 0x0000FF);
        break;
    default:
        break;
    }

    LED_signal++;
    if (LED_signal > 3)
    {
      LED_signal = 0;
    }
    
    delay(1000);
}
