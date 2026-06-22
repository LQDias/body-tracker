#include <Wire.h>
#include <math.h> // Biblioteca para garantir o uso do fabs()

const int MPU_ADDR = 0x68; 

int16_t rawAccX, rawAccY, rawAccZ;
int16_t rawGyroX, rawGyroY, rawGyroZ;

float accX, accY, accZ;
float gyroX, gyroY, gyroZ;

float angleX = 0.0; // Roll
float angleY = 0.0; // Pitch
float angleZ = 0.0; // Yaw

unsigned long lastTime;
float dt;

// Offsets para calibração
float gyroX_offset = 0.0; 
float gyroY_offset = 0.0; 
float gyroZ_offset = 0.0; 

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); 
  Wire.write(0);    
  Wire.endTransmission(true);

  Serial.println("Calibrando o sensor... DEIXE O SENSOR PARADO NA MESA!");
  delay(1000); // Dá 1 segundo para o usuário soltar o sensor
  
  long gyroX_sum = 0;
  long gyroY_sum = 0;
  long gyroZ_sum = 0;
  int validSamples = 0;
  
  while (validSamples < 400) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x43); 
    if (Wire.endTransmission(false) != 0) {
      delay(2);
      continue; // Falhou a transmissão, tenta de novo
    }
    
    Wire.requestFrom(MPU_ADDR, 6, true);
    if (Wire.available() < 6) {
      delay(2);
      continue; // Dados incompletos, descarta essa rodada
    }
    
    gyroX_sum += (int16_t)(Wire.read() << 8 | Wire.read());
    gyroY_sum += (int16_t)(Wire.read() << 8 | Wire.read());
    gyroZ_sum += (int16_t)(Wire.read() << 8 | Wire.read());
    validSamples++;
    delay(3);
  }
  
  gyroX_offset = (float)gyroX_sum / 400.0;
  gyroY_offset = (float)gyroY_sum / 400.0;
  gyroZ_offset = (float)gyroZ_sum / 400.0;
  
  Serial.print("Calibrado! Offset Z obtido: ");
  Serial.println(gyroZ_offset);

  lastTime = millis();
}

void loop() {
  unsigned long currentTime = millis();
  dt = (currentTime - lastTime) / 1000.0;
  lastTime = currentTime;

  // Evita dt zerado ou absurdo caso o loop rode rápido demais
  if (dt <= 0.0) dt = 0.001;

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); 
  if (Wire.endTransmission(false) != 0) {
    Serial.println("ERRO: MPU6050 nao respondeu!");
    delay(500);
    return;
  }
  
  Wire.requestFrom(MPU_ADDR, 14, true);
  if (Wire.available() < 14) {
    return;
  }

  rawAccX = Wire.read() << 8 | Wire.read();
  rawAccY = Wire.read() << 8 | Wire.read();
  rawAccZ = Wire.read() << 8 | Wire.read();
  Wire.read(); Wire.read(); // Pula temperatura
  rawGyroX = Wire.read() << 8 | Wire.read();
  rawGyroY = Wire.read() << 8 | Wire.read();
  rawGyroZ = Wire.read() << 8 | Wire.read();
  
  accX = (float)rawAccX / 16384.0;
  accY = (float)rawAccY / 16384.0;
  accZ = (float)rawAccZ / 16384.0;
  
  gyroX = ((float)rawGyroX - gyroX_offset) / 131.0;
  gyroY = ((float)rawGyroY - gyroY_offset) / 131.0;
  gyroZ = ((float)rawGyroZ - gyroZ_offset) / 131.0;

  // ZONA MORTA CORRIGIDA: Usando fabs() para float e tolerância de 1.5 graus/segundo
  if (fabs(gyroZ) < 1.5) {
    gyroZ = 0.0;
  }

  float accAngleX = atan2(accY, sqrt(accX * accX + accZ * accZ)) * 57.2958;
  float accAngleY = atan2(-accX, sqrt(accY * accY + accZ * accZ)) * 57.2958;

  // Filtro complementar para Roll e Pitch
  angleX = 0.98 * (angleX + gyroX * dt) + 0.02 * accAngleX;
  angleY = 0.98 * (angleY + gyroY * dt) + 0.02 * accAngleY;
  
  // Integração do Yaw limpa
  angleZ = angleZ + gyroZ * dt; 

  Serial.print("Roll:"); Serial.print(angleX); Serial.print(",");
  Serial.print("Pitch:"); Serial.print(angleY); Serial.print(",");
  Serial.print("Yaw:"); Serial.println(angleZ);
  
  delay(10); 
}