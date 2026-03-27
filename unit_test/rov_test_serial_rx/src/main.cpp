// подключаем библиотеки 
#include <Arduino.h>
#include <AsyncStream.h>
#include "config.h"


AsyncStream<200> serial1(&Serial1, '\n');

void setup() {
  // подключение отладочного сериала 
  Serial.begin(115200);

  Serial1.setRx(SERIAL1_RX);
  Serial1.setTx(SERIAL1_TX);
  Serial1.begin(SERIAL_BAUD);
}


void loop() {
  
  // если данные получены
  if (serial1.available()) {
    Serial.println(serial1.buf);
  }  

}
