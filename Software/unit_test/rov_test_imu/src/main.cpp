#include <Arduino.h>
#include <Wire.h>
#include "BNO085.h"

BNO085 bno08x;
bool bno085_connected = false;

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println(F("=== ROV IMU ДИАГНОСТИКА ==="));
  Serial.println(F("Платформа: STM32F103C8T6"));
  
  Wire.begin();
  Wire.setClock(100000);
  Serial.println(F("I2C инициализирован (100kHz)"));
  
  // Сканирование I2C
  Serial.println(F("Поиск BNO085..."));
  bool foundBNO085 = false;
  
  for (byte address = 0x4A; address <= 0x4B; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print(F("✓ BNO085 найден на 0x"));
      Serial.println(address, HEX);
      foundBNO085 = true;
      
      // Инициализация
      if (bno08x.begin(address)) {
        Serial.println(F("✓ BNO085 инициализирован"));
        bno085_connected = true;
        
        // Включаем сенсоры
        Serial.println(F("Включение сенсоров..."));
        
        if (bno08x.enableReport(SH2_ROTATION_VECTOR, 100000)) {
          Serial.println(F("✓ Rotation Vector включен"));
        }
        
        if (bno08x.enableReport(SH2_ACCELEROMETER, 100000)) {
          Serial.println(F("✓ Accelerometer включен"));
        }
        
        if (bno08x.enableReport(SH2_GYROSCOPE_CALIBRATED, 100000)) {
          Serial.println(F("✓ Gyroscope включен"));
        }
        
        if (bno08x.enableReport(SH2_MAGNETIC_FIELD_CALIBRATED, 100000)) {
          Serial.println(F("✓ Magnetometer включен"));
        }
        
        Serial.println(F("Готов к работе!"));
        break;
      } else {
        Serial.println(F("✗ Ошибка инициализации"));
      }
    }
  }
  
  if (!foundBNO085) {
    Serial.println(F("✗ BNO085 не найден!"));
    Serial.println(F("Проверьте подключение:"));
    Serial.println(F("- SDA -> PB7"));
    Serial.println(F("- SCL -> PB6"));
    Serial.println(F("- VCC -> 3.3V"));
    Serial.println(F("- GND -> GND"));
  }
  
  Serial.println(F("=========================================="));
}

void loop() {
  static unsigned long lastPrint = 0;
  static int readAttempts = 0;
  static int successfulReads = 0;
  
  if (bno085_connected) {
    sh2_SensorValue_t sensorValue;
    readAttempts++;
    
    if (bno08x.getSensorEvent(&sensorValue)) {
      successfulReads++;
      
      // Выводим данные
      switch (sensorValue.sensorId) {
        case SH2_ROTATION_VECTOR: {
          float roll, pitch, yaw;
          if (bno08x.getEulerAngles(&roll, &pitch, &yaw)) {
            Serial.print(F("Углы: Roll="));
            Serial.print(roll, 1);
            Serial.print(F("° Pitch="));
            Serial.print(pitch, 1);
            Serial.print(F("° Yaw="));
            Serial.print(yaw, 1);
            Serial.println(F("°"));
          }
          break;
        }
        
        case SH2_ACCELEROMETER:
          Serial.print(F("Акселерометр: X="));
          Serial.print(sensorValue.un.accelerometer.x, 2);
          Serial.print(F(" Y="));
          Serial.print(sensorValue.un.accelerometer.y, 2);
          Serial.print(F(" Z="));
          Serial.print(sensorValue.un.accelerometer.z, 2);
          Serial.println(F(" м/с²"));
          break;
          
        case SH2_GYROSCOPE_CALIBRATED:
          Serial.print(F("Гироскоп: X="));
          Serial.print(sensorValue.un.gyroscope.x, 3);
          Serial.print(F(" Y="));
          Serial.print(sensorValue.un.gyroscope.y, 3);
          Serial.print(F(" Z="));
          Serial.print(sensorValue.un.gyroscope.z, 3);
          Serial.println(F(" рад/с"));
          break;
          
        case SH2_MAGNETIC_FIELD_CALIBRATED:
          Serial.print(F("Магнитометр: X="));
          Serial.print(sensorValue.un.magneticField.x, 1);
          Serial.print(F(" Y="));
          Serial.print(sensorValue.un.magneticField.y, 1);
          Serial.print(F(" Z="));
          Serial.print(sensorValue.un.magneticField.z, 1);
          Serial.println(F(" мкТл"));
          break;
      }
    }
  }
  
  // Статистика каждые 5 секунд
  if (millis() - lastPrint > 5000) {
    if (bno085_connected) {
      Serial.print(F("Статистика: Попыток="));
      Serial.print(readAttempts);
      Serial.print(F(" Успешных="));
      Serial.print(successfulReads);
      Serial.print(F(" Успех="));
      if (readAttempts > 0) {
        Serial.print((float)successfulReads / readAttempts * 100, 1);
        Serial.println(F("%"));
      } else {
        Serial.println(F("0%"));
      }
    } else {
      Serial.println(F("BNO085 не подключен"));
    }
    lastPrint = millis();
  }
  
  delay(10);
}