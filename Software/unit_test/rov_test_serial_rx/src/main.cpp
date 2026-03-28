// Приём строки по Serial1 до '\n', вывод в USB Serial через StringUtils::Text.
#include <Arduino.h>
#include <StringUtils.h>
#include "config.h"

static char lineBuf[200];
static size_t lineLen = 0;

void setup() {
  Serial.begin(115200);

  Serial1.setRx(SERIAL1_RX);
  Serial1.setTx(SERIAL1_TX);
  Serial1.begin(SERIAL_BAUD);
}

void loop() {
  while (Serial1.available()) {
    const char c = static_cast<char>(Serial1.read());
    if (c == '\n') {
      if (lineLen < sizeof(lineBuf)) {
        lineBuf[lineLen] = '\0';
      } else {
        lineBuf[sizeof(lineBuf) - 1] = '\0';
      }
      const Text line(lineBuf);
      line.printTo(Serial);
      Serial.println();
      lineLen = 0;
    } else if (c != '\r') {
      if (lineLen < sizeof(lineBuf) - 1) {
        lineBuf[lineLen++] = c;
      } else {
        lineLen = 0;
      }
    }
  }
}
