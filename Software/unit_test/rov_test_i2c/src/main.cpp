#include <Arduino.h>
#include <Wire.h>

// Пины для UART телеметрии (USART1)
#define UART_RX PA10
#define UART_TX PA9
#define SERIAL_BAUD_RATE 115200

// Пины для I2C
#define I2C_SDA_PIN PB7
#define I2C_SCL_PIN PB6

// UART для вывода результатов
#define TelemetrySerial Serial

void setup() {
  // Инициализация UART для телеметрии на PA9/PA10
  TelemetrySerial.setRx(UART_RX);
  TelemetrySerial.setTx(UART_TX);
  TelemetrySerial.begin(SERIAL_BAUD_RATE);
  while (!TelemetrySerial) delay(10); // Ждем готовности UART порта
  
  // Инициализация I2C
  Wire.setSDA(I2C_SDA_PIN);
  Wire.setSCL(I2C_SCL_PIN);
  Wire.begin();
  
  TelemetrySerial.println("\n=== I2C Scanner ===");
  TelemetrySerial.println("Scanning I2C bus...");
  TelemetrySerial.println();
  
  delay(1000);
}

void loop() {
  byte error, address;
  int nDevices = 0;
  
  TelemetrySerial.println("Scanning...");
  
  // Сканируем адреса от 0x08 до 0x77 (стандартный диапазон I2C)
  for (address = 0x08; address < 0x78; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      TelemetrySerial.print("I2C device found at address 0x");
      if (address < 0x10) {
        TelemetrySerial.print("0");
      }
      TelemetrySerial.print(address, HEX);
      TelemetrySerial.print(" (");
      TelemetrySerial.print(address, DEC);
      TelemetrySerial.println(")");
      nDevices++;
    } else if (error == 4) {
      TelemetrySerial.print("Unknown error at address 0x");
      if (address < 0x10) {
        TelemetrySerial.print("0");
      }
      TelemetrySerial.println(address, HEX);
    }
  }
  
  if (nDevices == 0) {
    TelemetrySerial.println("No I2C devices found");
  } else {
    TelemetrySerial.print("Found ");
    TelemetrySerial.print(nDevices);
    TelemetrySerial.println(" device(s)");
  }
  
  TelemetrySerial.println();
  TelemetrySerial.println("---");
  TelemetrySerial.println();
  
  delay(1000); // Повторяем сканирование каждые 5 секунд
}
