#include <Arduino.h>
#include <Servo.h>

Servo servo;
const int SERVO_PIN = 3;

// Параметры для агрессивного движения
const int MIN_POSITION = 1200;    // Минимальная позиция
const int MAX_POSITION = 1800;    // Максимальная позиция
const int CENTER_POSITION = 1500; // Центральная позиция
const int STEP_DELAY = 15;        // Задержка между шагами (мс) - уменьшено для скорости
const int STEP_SIZE = 20;         // Размер шага для агрессивного движения - увеличено

int currentPosition = CENTER_POSITION;

void setup() {
  // Подключение сервопривода (1000-2000 мкс)
  servo.attach(SERVO_PIN, 1000, 2000);

  servo.writeMicroseconds(CENTER_POSITION);  // Центр
  delay(2000);
}

// Функция для быстрого движения к целевой позиции
void smoothMoveTo(int targetPosition) {
  while (currentPosition != targetPosition) {
    if (currentPosition < targetPosition) {
      currentPosition += STEP_SIZE;
      if (currentPosition > targetPosition) {
        currentPosition = targetPosition;
      }
    } else {
      currentPosition -= STEP_SIZE;
      if (currentPosition < targetPosition) {
        currentPosition = targetPosition;
      }
    }
    
    servo.writeMicroseconds(currentPosition);
    delay(STEP_DELAY);
  }
}

void loop() {
  // Быстрое движение к минимуму
  smoothMoveTo(MIN_POSITION);
  delay(10000);
  
  // Быстрое движение к центру
  smoothMoveTo(CENTER_POSITION);
  delay(1000);
  
  // Быстрое движение к максимуму
  smoothMoveTo(MAX_POSITION);
  delay(10000);

  // Быстрое движение к центру
  smoothMoveTo(CENTER_POSITION);
  delay(1000);
}
