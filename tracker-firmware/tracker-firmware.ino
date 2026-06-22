#include <Wire.h>

// Endereço I2C do MPU6050
const int MPU_ADDR = 0x68; 

// Variáveis para dados brutos
int16_t accelX, accelY, accelZ;
int16_t gyroX, gyroY, gyroZ;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  // Inicializa o MPU6050 (Acorda o sensor)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // Registrador de gerenciamento de energia (Power Management 1)
  Wire.write(0);    // Define como 0 para acordar o sensor
  Wire.endTransmission(true);
}

void loop() {
  // Aponta para o primeiro registrador de dados (Accel X High)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); 
  Wire.endTransmission(false);
  
  // Solicita 14 bytes (6 para acel, 2 para temp, 6 para giro)
  Wire.requestFrom(MPU_ADDR, 14, true);
  
  // Leitura dos dados combinando High e Low Byte
  accelX = Wire.read() << 8 | Wire.read();
  accelY = Wire.read() << 8 | Wire.read();
  accelZ = Wire.read() << 8 | Wire.read();
  
  int16_t temp = Wire.read() << 8 | Wire.read(); // Temperatura (ignorada por enquanto)
  
  gyroX = Wire.read() << 8 | Wire.read();
  gyroY = Wire.read() << 8 | Wire.read();
  gyroZ = Wire.read() << 8 | Wire.read();
  
  // Printa no Serial Plotter/Monitor para testar
  Serial.print("AccX:"); Serial.print(accelX); Serial.print(",");
  Serial.print("AccY:"); Serial.print(accelY); Serial.print(",");
  Serial.print("GyroX:"); Serial.println(gyroX);
  
  delay(10); // Taxa de amostragem inicial de ~100Hz
}