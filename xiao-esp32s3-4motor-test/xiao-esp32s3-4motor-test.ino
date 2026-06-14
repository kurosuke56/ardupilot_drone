const int M1 = 1;   // GPIO1
const int M2 = 2;   // GPIO2
const int M3 = 7;   // GPIO7
const int M4 = 8;   // GPIO8

const int PWM_FREQ = 20000;   // 20kHz
const int PWM_RES  = 8;       // 0～255

const int LED_PIN = 21;  // XIAO ESP32S3の内蔵LED

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);	// ピンの電圧をLOWにする（実際は点灯）
  
  ledcAttach(M1, PWM_FREQ, PWM_RES);
  ledcAttach(M2, PWM_FREQ, PWM_RES);
  ledcAttach(M3, PWM_FREQ, PWM_RES);
  ledcAttach(M4, PWM_FREQ, PWM_RES);

  stopAll();

  Serial.println("Motor test start");
}

void loop() {
  testMotor(M1, "M1");
  testMotor(M2, "M2");
  testMotor(M3, "M3");
  testMotor(M4, "M4");
}

void testMotor(int pin, const char* name) {

  Serial.print("Testing ");
  Serial.println(name);

  // 徐々に加速
  for (int duty = 0; duty <= 255; duty += 5) {
    ledcWrite(pin, duty);
    delay(50);
  }

  delay(200);

  // 徐々に減速
  for (int duty = 255; duty >= 0; duty -= 5) {
    ledcWrite(pin, duty);
    delay(50);
  }

  ledcWrite(pin, 0);

  delay(200);
}

void stopAll() {
  ledcWrite(M1, 0);
  ledcWrite(M2, 0);
  ledcWrite(M3, 0);
  ledcWrite(M4, 0);
}