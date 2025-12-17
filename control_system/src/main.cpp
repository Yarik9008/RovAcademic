// подключаем библиотеки 
#include <Arduino.h>
#include <Servo.h>
#include <GParser.h>
#include <AsyncStream.h>
#include <ServoSmooth.h>
#include <config.h>


ServoSmooth servos[6];

AsyncStream<100> serialCom(&Serial1, '\n');

uint32_t turnTimer = 0;
uint32_t ledTimer = 0;
uint32_t lastDataTime = 0;  // время последнего получения данных
int ledState = LOW;

// Массив для отправки данных на полезную нагрузку
// [MOTOR1, MOTOR2, MOTOR3, MOTOR4, CAM, GRIP]
int data_output[6] = {SERVO_CENTER, SERVO_CENTER, SERVO_CENTER, SERVO_CENTER, SERVO_CENTER, GRIP_CLOSE};
// значения по умолчанию
const int data_output_default[6] = {SERVO_CENTER, SERVO_CENTER, SERVO_CENTER, SERVO_CENTER, SERVO_CENTER, GRIP_CLOSE};


void setup() {
  // подключение светодиода для индикации работы
  pinMode(LED_PIN, OUTPUT);

  // подключение отладочного сериала 
  Serial.begin(BITRATE);

  // подключение сериала для общения с постом управления 
  Serial1.setRx(UART_RX);
  Serial1.setTx(UART_TX);
  Serial1.begin(BITRATE);
  
  // инициализация таймера получения данных
  lastDataTime = millis();
  
  // подключаем моторы 
  const uint16_t motor_pins[4] = {PIN_MOTOR_1, PIN_MOTOR_2, PIN_MOTOR_3, PIN_MOTOR_4};
  for (int i = 0; i < 4; i++) {
    servos[i].attach(motor_pins[i], SERVO_MIN, SERVO_MAX, 90);
    servos[i].setDirection(MOTOR_INVERT[i]);
    servos[i].setSpeed(SPEED_MOTORS);
    servos[i].setAccel(ACCEL_MOTORS);
    servos[i].setAutoDetach(false);
  }

  // подключаем сервопривод камеры
  servos[4].attach(PIN_SERVO_CAM, SERVO_MIN, SERVO_MAX, 90);
  servos[4].setSpeed(SPEED_SERVO);
  servos[4].setAccel(ACCELERATE_SERVO);
  servos[4].writeMicroseconds(SERVO_CENTER);
  servos[4].setAutoDetach(true);

  // подключаем сервопривод манипулятора
  servos[5].attach(PIN_SERVO_ARM, SERVO_MIN, SERVO_MAX, ARM_INIT_ANGLE);
  servos[5].setSpeed(ARM_SPEED);
  servos[5].setAccel(ARM_ACCEL);
  servos[5].setTarget(GRIP_CLOSE);
  servos[5].setAutoDetach(true);

  // Задержка для инициализации моторов
  delay(3000);
}

// главный цикл работы
void loop() {
  // проверка таймаута получения данных
  if (millis() - lastDataTime >= DATA_TIMEOUT) {
    // если данные не получены в течение таймаута, устанавливаем значения по умолчанию
    for (int i = 0; i < 6; i++) {
      data_output[i] = data_output_default[i];
      servos[i].setTarget(data_output[i]);
    }
  }
  
  // обновление сервоприводов
  if (millis() - turnTimer >= SERVO_UPDATE_INTERVAL){
    turnTimer = millis();
    for (int i = 0; i < 6; i++) {
      servos[i].tick();
    }
  }

  // мигалка для индикации работы
  if (millis() - ledTimer >= LED_BLINK_INTERVAL){
    ledTimer = millis();
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }
    
  // если данные получены
  if (serialCom.available()) {
    // парсим данные по разделителю, возвращает список интов 
    GParser data = GParser(serialCom.buf, ' ');

    if (DEBUG) {
      Serial.print("Received: ");
      Serial.println(serialCom.buf);
    }

    if (data.amount() == EXPECTED_DATA_COUNT){
      // обновляем время последнего получения данных только при успешном парсинге
      lastDataTime = millis();
      
      // Формат входных данных: joy1X joy1Y joy2X joy2Y CAM GRIP LED B7 B8
      // Используем статический массив вместо VLA для безопасности
      int data_input[INPUT_DATA_BUFFER_SIZE];
      int parsed_count = data.parseInts(data_input);
      
      // Проверка успешности парсинга
      if (parsed_count != EXPECTED_DATA_COUNT) {
        if (DEBUG) {
          Serial.print("Parse error: expected ");
          Serial.print(EXPECTED_DATA_COUNT);
          Serial.print(", got ");
          Serial.println(parsed_count);
        }
        return;  // Пропускаем обработку при ошибке парсинга
      }

      // Валидация входных данных (ожидаем значения джойстиков в диапазоне 1000-2000)
      bool data_valid = true;
      for (int i = 0; i < 4; i++) {
        if (data_input[i] < SERVO_MIN || data_input[i] > SERVO_MAX) {
          data_valid = false;
          break;
        }
      }
      
      if (!data_valid) {
        if (DEBUG) {
          Serial.println("Invalid input data range");
        }
        return;  // Пропускаем обработку при невалидных данных
      }

      // Формируем управляющие значения для полезной нагрузки
      // [MOTOR1, MOTOR2, MOTOR3, MOTOR4, CAM, GRIP]
      // Дифференциальное управление моторами на основе джойстиков
      data_output[0] = data_input[3] + data_input[2] - SERVO_CENTER;  // MOTOR1: joy2X + joy2Y
      data_output[1] = data_input[3] - data_input[2] + SERVO_CENTER;  // MOTOR2: joy2X - joy2Y
      data_output[2] = data_input[1];  // MOTOR3: прямое значение joy1Y
      data_output[3] = (SERVO_MAX * 2) - data_input[1];  // MOTOR4: инвертированное joy1Y

      // Управление камерой: накопительное изменение позиции
      int16_t cam_delta = data_input[4] * CAM_STEP;
      data_output[4] = constrain(data_output[4] + cam_delta, SERVO_MIN, SERVO_MAX);

      // Управление манипулятором: дискретные состояния
      if (data_input[5] == 1) {
        data_output[5] = GRIP_OPEN;
      } else if (data_input[5] == -1) {
        data_output[5] = GRIP_CLOSE;
      }
      // Если data_input[5] == 0, оставляем текущее значение
    
      // Проверка и ограничение значений в диапазоне SERVO_MIN-SERVO_MAX
      for (int i = 0; i < 6; i++) {
        data_output[i] = constrain(data_output[i], SERVO_MIN, SERVO_MAX);
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