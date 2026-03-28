/* Пример использования оптимизированной библиотеки MS5837
-----------------------------------------------------

Описание: Минимальный пример с оптимизированной библиотекой MS5837
для измерения глубины. Только необходимый функционал.

*/

#include <Wire.h>
#include "MS5837.h"

MS5837 sensor;

// Переменные для хранения предыдущих значений
float lastPressure = 0.0;
float lastTemperature = 0.0;
float lastDepth = 0.0;
bool hasValidData = false;

void setup() {
  Serial.begin(115200);
  
  Serial.println("=== Оптимизированная библиотека MS5837 ===");
  
  Wire.begin();

  // Инициализация датчика
  sensor.init();
  sensor.setFluidDensity(997); // Пресная вода
  
  Serial.println("Инициализация датчика...");
  
  // Ждем готовности датчика
  while (!sensor.isReady()) {
    sensor.continueInit();
    delay(100);
  }
  
  Serial.println("Датчик готов!");
  
  // Калибровка нулевой глубины
  Serial.println("Калибровка нулевой глубины...");
  
  // Ждем первого измерения для калибровки
  while (!sensor.read()) {
    delay(10);
  }
  
  // Выполняем калибровку
  sensor.calibrateSurface();
  Serial.print("Калибровка завершена! Поверхностное давление: ");
  Serial.print(sensor.pressure(), 2);
  Serial.println(" мбар");
  
  Serial.println("\nНачинаем измерения:");
  Serial.println("Давление\tТемпература\tГлубина");
  Serial.println("(мбар)\t\t(°C)\t\t(м)");
  Serial.println("----------------------------------------");
}

void loop() {
  // Неблокирующее чтение датчика
  if (sensor.read()) {
    // Чтение завершено, сохраняем новые данные
    lastPressure = sensor.pressure();
    lastTemperature = sensor.temperature();
    lastDepth = sensor.depth();
    
    hasValidData = true;
    
    // Выводим новые данные
    Serial.print(lastPressure, 1);
    Serial.print("\t\t");
    Serial.print(lastTemperature, 1);
    Serial.print("\t\t");
    Serial.print(lastDepth, 2);
    Serial.println();
  } else {
    // Чтение не завершено, выводим предыдущие данные
    if (hasValidData) {
      Serial.print(lastPressure, 1);
      Serial.print("\t\t");
      Serial.print(lastTemperature, 1);
      Serial.print("\t\t");
      Serial.print(lastDepth, 2);
      Serial.println();
    }
  }
}
