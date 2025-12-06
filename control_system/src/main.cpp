// подключаем библиотеки 
#include <Arduino.h>
#include <Servo.h>
#include <GParser.h>
#include <AsyncStream.h>
#include <ServoSmooth.h>
#include <Config.h>
#include <GyverFilters.h>


ServoSmooth servos[6];

AsyncStream<100> serialCom(&Serial1, '\n');

uint32_t turnTimer;
uint32_t ledTimer;
int ledState = LOW;

// Массив для отправки данных на полезную нагрузку
// [MOTOR1, MOTOR2, MOTOR3, MOTOR4, CAM, GRIP]
int data_output[6] = {1500, 1500, 1500, 1500, 1500, 1300};


void setup() {
  // подключение светодиода для индикации работы
  pinMode(LED_PIN, OUTPUT);

  // подключение отладочного сериала 
  Serial.begin(BITRATE);

  // подключение сериала для общения с постом управления 
  Serial1.setRx(UART_RX);
  Serial1.setTx(UART_TX);
  Serial1.begin(BITRATE);
  
  // подключаем моторы 
  const uint16_t motor_pins[4] = {PIN_MOTOR_1, PIN_MOTOR_2, PIN_MOTOR_3, PIN_MOTOR_4};
  for (int i = 0; i < 4; i++) {
    servos[i].attach(motor_pins[i], 1000, 2000, 90);
    servos[i].setDirection(MOTOR_INVERT[i]);
    servos[i].setSpeed(SPEED_MOTORS);
    servos[i].setAccel(ACCEL_MOTORS);
    servos[i].setAutoDetach(false);
  }

  // подключаем сервопривод камеры
  servos[4].attach(PIN_SERVO_CAM, 1000, 2000, 90);
  servos[4].setSpeed(SPEED_SERVO);
  servos[4].setAccel(ACCELERATE_SERVO);
  servos[4].writeMicroseconds(1500);
  servos[4].setAutoDetach(true);

  // подключаем сервопривод манипулятора
  servos[5].attach(PIN_SERVO_ARM, 1000, 2000, 52);
  servos[5].setSpeed(200);
  servos[5].setAccel(1.0);
  servos[5].setTarget(1300);
  servos[5].setAutoDetach(true);

}

// главный цикл работы
void loop() {
  // обновление сервоприводов
  if (millis()- turnTimer >= 10){
    turnTimer = millis();
    for (int i = 0; i < 6; i++) {
      servos[i].tick();
    }
  }

  // мигалка для индикации работы (период 500 мс)
  if (millis() - ledTimer >= 500){
    ledTimer = millis();
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    // if (DEBUG) Serial.println(ledState);
  }
    
  // если данные получены
  if (serialCom.available()) {

    // парсим данные по резделителю возвращает список интов 
    GParser data = GParser(serialCom.buf, ' ');

    // if (DEBUG) Serial.println(serialCom.buf);

    if (data.amount() == 9){
      // 1500 1500 1500 1500 1 1 1 1 1
      // joy1X joy1Y joy2X joy2Y CAM GRIP LED B7 B8
      int data_input[data.amount()];
      int am = data.parseInts(data_input);

      // формируем считаем управляющие значения для полезной нагрузки
      // [MOTOR1, MOTOR2, MOTOR3, MOTOR4, CAM, GRIP, LED, B7, B8]
      data_output[0] = data_input[3] + data_input[2] - 1500;  // MOTOR1
      data_output[1] = data_input[3] - data_input[2] + 1500;  // MOTOR2
      data_output[2] = data_input[1];  // MOTOR3
      data_output[3] = 3000 - data_input[1];  // MOTOR4

      data_output[4] = data_output[4] + data_input[4] * 20 ;  // CAM

      if (data_input[5] == 1) {
        data_output[5] = 2000;
      } else if (data_input[5] == -1) {
        data_output[5] = 1300;
      } 
    
      // проверка и ограничение значений в диапазоне 1000-2000
      for (int i = 0; i < 6; i++) {
        if (data_output[i] < 1000) {
          data_output[i] = 1000;
        } else if (data_output[i] > 2000) {
          data_output[i] = 2000;
        }
      }

      if (DEBUG) {
        Serial.print(data_output[0]); Serial.print(" ");
        Serial.print(data_output[1]); Serial.print(" ");
        Serial.print(data_output[2]); Serial.print(" ");
        Serial.print(data_output[3]); Serial.print(" ");
        Serial.print(data_output[4]); Serial.print(" ");
        Serial.println(data_output[5]);
      }

      // отправляем значения на полезную нагрузку
      for (int i = 0; i < 6; i++) {
        servos[i].setTarget(data_output[i]);
      }  

    }
  }  
}