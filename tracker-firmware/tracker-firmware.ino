#include <Wire.h>
#include <math.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "secrets.h"

// ========== CONFIGURAÇÃO DO WI-FI E REDE ==========
const char* ssid     = SECRET_SSID; 
const char* password = SECRET_PASSWORD;
const char* computerIP = SECRET_COMP_IP; 
const unsigned int localPort = 2390;      
const unsigned int remotePort = 4242;     

WiFiUDP Udp;

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

float gyroX_offset = 0.0; 
float gyroY_offset = 0.0; 
float gyroZ_offset = 0.0; 

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  // 1. Inicializa o MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); 
  Wire.write(0);    
  Wire.endTransmission(true);

  // 2. Conecta ao Wi-Fi
  Serial.print("Conectando em: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Conectado!");
  
  // NOVA TRAVA: Espera até o roteador entregar um IP real
  while (WiFi.localIP()[0] == 0) {
    delay(500);
    Serial.print("Aguardando IP do roteador...");
  }

  Serial.print("IP do Arduino: ");
  Serial.println(WiFi.localIP());

  Udp.begin(localPort);

  // 4. Calibração
  Serial.println("Calibrando o sensor... NAO MEXA!");
  long gyroX_sum = 0, gyroY_sum = 0, gyroZ_sum = 0;
  int validSamples = 0;
  
  while (validSamples < 400) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x43); 
    if (Wire.endTransmission(false) != 0) { delay(2); continue; }
    
    Wire.requestFrom(MPU_ADDR, 6, true);
    if (Wire.available() < 6) { delay(2); continue; }
    
    gyroX_sum += (int16_t)(Wire.read() << 8 | Wire.read());
    gyroY_sum += (int16_t)(Wire.read() << 8 | Wire.read());
    gyroZ_sum += (int16_t)(Wire.read() << 8 | Wire.read());
    validSamples++;
    delay(3);
  }
  
  gyroX_offset = (float)gyroX_sum / 400.0;
  gyroY_offset = (float)gyroY_sum / 400.0;
  gyroZ_offset = (float)gyroZ_sum / 400.0;
  
  Serial.println("Calibrado e pronto para transmitir!");
  lastTime = millis();
}

void loop() {
  unsigned long currentTime = millis();
  dt = (currentTime - lastTime) / 1000.0;
  lastTime = currentTime;
  if (dt <= 0.0) dt = 0.001;

  // Leitura do MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); 
  if (Wire.endTransmission(false) != 0) return;
  
  Wire.requestFrom(MPU_ADDR, 14, true);
  if (Wire.available() < 14) return;

  rawAccX = Wire.read() << 8 | Wire.read();
  rawAccY = Wire.read() << 8 | Wire.read();
  rawAccZ = Wire.read() << 8 | Wire.read();
  Wire.read(); Wire.read(); 
  rawGyroX = Wire.read() << 8 | Wire.read();
  rawGyroY = Wire.read() << 8 | Wire.read();
  rawGyroZ = Wire.read() << 8 | Wire.read();
  
  accX = (float)rawAccX / 16384.0;
  accY = (float)rawAccY / 16384.0;
  accZ = (float)rawAccZ / 16384.0;
  
  gyroX = ((float)rawGyroX - gyroX_offset) / 131.0;
  gyroY = ((float)rawGyroY - gyroY_offset) / 131.0;
  gyroZ = ((float)rawGyroZ - gyroZ_offset) / 131.0;

  if (fabs(gyroZ) < 1.5) gyroZ = 0.0;

  float accAngleX = atan2(accY, sqrt(accX * accX + accZ * accZ)) * 57.2958;
  float accAngleY = atan2(-accX, sqrt(accY * accY + accZ * accZ)) * 57.2958;

  angleX = 0.98 * (angleX + gyroX * dt) + 0.02 * accAngleX;
  angleY = 0.98 * (angleY + gyroY * dt) + 0.02 * accAngleY;
  angleZ = angleZ + gyroZ * dt; 

  // ================= TRANSMISSÃO SEM FIO (UDP) =================
  String packetData = String(angleX) + "," + String(angleY) + "," + String(angleZ);
  
  Udp.beginPacket(computerIP, remotePort);
  Udp.print(packetData);
  Udp.endPacket();
  // =============================================================

  Serial.println(packetData);
  
  delay(10); // Envia pacotes a aprox. 100Hz
}