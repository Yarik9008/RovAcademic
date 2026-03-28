#include <Arduino.h>
#include <Servo.h>

Servo servo;
const int SERVO_PIN = PB1;

void setup() {
  // Подключение сервопривода (1000-2000 мкс)
  servo.attach(SERVO_PIN, 1000, 2000);
}

void loop() {
  servo.writeMicroseconds(1000);  // Минимум
  delay(5000);
  
  servo.writeMicroseconds(1500);  // Центр
  delay(5000);
  
  servo.writeMicroseconds(2000);  // Максимум
  delay(5000);

  servo.writeMicroseconds(1500);  // Центр
  delay(5000);
}
