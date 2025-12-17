#include <Arduino.h>

// Встроенный LED на Blue Pill находится на пине PC13
#define LED_PIN PC13

void setup() {
  // Инициализация Serial для вывода в терминал
  Serial.begin(115200);
  
  // Инициализация пина LED как выход
  pinMode(LED_PIN, OUTPUT);
  
  Serial.println("Blink test started");
}

void loop() {
  // Включаем LED
  digitalWrite(LED_PIN, LOW);  // На Blue Pill LED активен при LOW
  Serial.println("LED ON");
  delay(500);                   // Ждем 500 мс
  
  // Выключаем LED
  digitalWrite(LED_PIN, HIGH); // Выключаем LED
  Serial.println("LED OFF");
  delay(500);                   // Ждем 500 мс
}