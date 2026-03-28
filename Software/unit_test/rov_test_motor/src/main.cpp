#include <Arduino.h>
#include "ROVMotor.h"

// Пины для моторов
const int MOTOR_PINS[] = {3, 5, 6, 9}; // 4 мотора на разных пинах
const int MOTOR_COUNT = 4;

void setup() {
  Serial.begin(9600);
  Serial.println("ROV Control System - Multiple BLDC Motors");
  
  // Добавление моторов в статический массив
  for (int i = 0; i < MOTOR_COUNT; i++) {
    if (ROVMotor::addMotor(MOTOR_PINS[i])) {
      Serial.print("Motor ");
      Serial.print(i);
      Serial.print(" added on pin ");
      Serial.println(MOTOR_PINS[i]);
    } else {
      Serial.print("Failed to add motor ");
      Serial.println(i);
    }
  }
  
  // Инициализация всех моторов
  ROVMotor::beginAll();
  
  Serial.print("Total motors: ");
  Serial.println(ROVMotor::getMotorCount());
  
  delay(2000);
}

void loop() {
  // Обновление всех моторов с встроенным таймером (в стиле ServoSmooth)
  ROVMotor::tickAll();
  
  static unsigned long lastCommandTime = 0;
  static int commandStep = 0;
  
  if (millis() - lastCommandTime >= 8000) {
    switch (commandStep) {
      case 0: 
        // Движение вперед - все моторы на 50% мощности
        ROVMotor::setAllMotorsPercent(50);
        Serial.println("All motors forward 50%");
        break;
        
      case 1: 
        // Поворот влево - разные скорости моторов в процентах
        ROVMotor::setMotorPercent(0, 30); // Левый мотор медленнее
        ROVMotor::setMotorPercent(1, 70); // Правый мотор быстрее
        ROVMotor::setMotorPercent(2, 30); // Левый мотор медленнее
        ROVMotor::setMotorPercent(3, 70); // Правый мотор быстрее
        Serial.println("Turn left");
        break;
        
      case 2: 
        // Поворот вправо - разные скорости моторов в процентах
        ROVMotor::setMotorPercent(0, 70); // Левый мотор быстрее
        ROVMotor::setMotorPercent(1, 30); // Правый мотор медленнее
        ROVMotor::setMotorPercent(2, 70); // Левый мотор быстрее
        ROVMotor::setMotorPercent(3, 30); // Правый мотор медленнее
        Serial.println("Turn right");
        break;
        
      case 3: 
        // Остановка всех моторов
        ROVMotor::stopAllMotors();
        Serial.println("All motors stop");
        break;
    }
    commandStep = (commandStep + 1) % 4;
    lastCommandTime = millis();
  }
}
