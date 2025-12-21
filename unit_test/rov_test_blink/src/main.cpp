#include <Arduino.h>

// Встроенный LED на Blue Pill находится на пине PC13
#define LED_PIN PC13

void setup() {
  // Инициализация Serial для вывода в терминал
  Serial.begin(115200);

  // Инициализация светодиода
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // Включаем LED
  digitalWrite(LED_PIN, LOW);  
  Serial.println("LED ON");
  delay(500);                 
  
  // Выключаем LED
  digitalWrite(LED_PIN, HIGH); 
  Serial.println("LED OFF");
  delay(500);                  
}