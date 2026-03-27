#include <Arduino.h>
#include "config.h"

uint16_t count1 = 0;
unsigned long previousMillis = 0;

void setup() {
  // инициализация отладочного сериала 
  Serial.begin(115200);
  
  // инициализация сериала для отправки на основной пульт 
  Serial1.setRx(SERIAL1_RX);
  Serial1.setTx(SERIAL1_TX);

  Serial1.begin(SERIAL_BAUD);
}

void loop() {
  unsigned long currentMillis = millis();
  
  // проверяем, прошло ли достаточно времени с последней отправки
  if (currentMillis - previousMillis >= TIMEOUT) {
    // отправка строки 
    Serial1.println(count1);
    Serial.print("send ");
    Serial.println(count1);
    count1 = count1 + 1;
    
    // обновляем время последней отправки
    previousMillis = currentMillis;
  }
}