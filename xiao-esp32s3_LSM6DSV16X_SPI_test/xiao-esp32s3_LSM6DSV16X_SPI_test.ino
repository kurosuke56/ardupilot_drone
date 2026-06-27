#include <LSM6DSV16XSensor.h>

////////////////////////////////////////
// Pin definition
////////////////////////////////////////

#define PIN_CS 44   // D7
#define PIN_SCK 7   // D8
#define PIN_MISO 8  // D9
#define PIN_MOSI 9  // D10

//#define USE_HSPI_PORT

SPIClass SPI_IMU(SPI);
LSM6DSV16XSensor imu(&SPI_IMU, PIN_CS);

void setup() {
  Serial.begin(115200);
  Serial.println();

  delay(1000);

  SPI_IMU.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);
  SPI_IMU.setFrequency(1000000);  // 1MHz

  imu.begin();
  imu.Enable_X();
  imu.Enable_G();
}

void loop() {
  int32_t acc[3];
  int32_t gyro[3];

  imu.Get_X_Axes(acc);
  imu.Get_G_Axes(gyro);

  Serial.print("ACC: ");

  Serial.print(acc[0]);
  Serial.print(",");
  Serial.print(acc[1]);
  Serial.print(",");
  Serial.print(acc[2]);

  Serial.print("    ");

  Serial.print("GYRO: ");

  Serial.print(gyro[0]);
  Serial.print(",");
  Serial.print(gyro[1]);
  Serial.print(",");
  Serial.println(gyro[2]);

  delay(100);
}