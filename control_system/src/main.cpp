/**
 * @file main.cpp
 * Плата управления: приём команд по UART от геймпада, смешивание моторов, серво камеры и захвата.
 *
 * Цикл: таймаут связи → безопасные значения; иначе парсинг пакета → валидация → обновление выходов.
 * Поля B7 и B8 проверяются по диапазону, на плате не задействованы (резерв протокола с gamepad).
 */

#include <Arduino.h>
#include <Servo.h>
#include <GParser.h>
#include <AsyncStream.h>
#include <ServoSmooth.h>

#include "config.h"

// ---------------------------------------------------------------------------
// Глобальное состояние
// ---------------------------------------------------------------------------

ServoSmooth servos[NUM_CHANNELS];
AsyncStream<100> serialCom(&Serial1, '\n');

static uint32_t turnTimer = 0;
static uint32_t ledTimer = 0;
static uint32_t lastDataTime = 0;
static int ledState = LOW;

/**
 * Режим индикации LED_PIN: 0 — мигание (есть связь по пакетам);
 * −1 / +1 — фиксированный уровень с поля LED пульта.
 */
static int8_t ledUiMode = 0;

/** Уже вошли в fail-safe по таймауту (не дёргать приводы каждый кадр повторно). */
static bool failSafeLatched = false;

/** Целевые PWM (мкс) на приводы: [M1, M2, M3, M4, CAM, GRIP]. */
static int data_output[NUM_CHANNELS] = {
    SERVO_CENTER, SERVO_CENTER, SERVO_CENTER, SERVO_CENTER, SERVO_CENTER, GRIP_CLOSE};

static const int data_output_default[NUM_CHANNELS] = {
    SERVO_CENTER, SERVO_CENTER, SERVO_CENTER, SERVO_CENTER, SERVO_CENTER, GRIP_CLOSE};

// ---------------------------------------------------------------------------
// Вспомогательные функции
// ---------------------------------------------------------------------------

/** Записать цели в ServoSmooth; захват — без сглаживания (writeMicroseconds). */
static void applyServoTargets() {
  for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
    if (i == GRIP_CHANNEL_INDEX) {
      servos[GRIP_CHANNEL_INDEX].writeMicroseconds((uint16_t)data_output[GRIP_CHANNEL_INDEX]);
    } else {
      servos[i].setTarget(data_output[i]);
    }
  }
}

/**
 * Проверка диапазонов полей пакета (согласовано с прошивкой gamepad).
 * @return true если все поля допустимы.
 */
static bool packetDataValid(const int* d) {
  for (int i = 0; i < 4; i++) {
    if (d[i] < (int)SERVO_MIN || d[i] > (int)SERVO_MAX) {
      return false;
    }
  }
  for (int j = 4; j <= 6; j++) {
    if (d[j] < -1 || d[j] > 1) {
      return false;
    }
  }
  if (d[7] != 0 && d[7] != 1) {
    return false;
  }
  if (d[8] != 0 && d[8] != 1) {
    return false;
  }
  return true;
}

/**
 * Заполнить data_output из распарсенного пакета (после packetDataValid).
 * Моторы: дифференциал по второму стику; M3/M4 — вертикаль первого стика.
 */
static void fillOutputsFromPacket(const int* in) {
  data_output[0] = in[3] + in[2] - SERVO_CENTER;
  data_output[1] = in[3] - in[2] + SERVO_CENTER;
  data_output[2] = in[1];
  data_output[3] = (SERVO_CENTER * 2) - in[1];

  const int16_t cam_delta = in[4] * CAM_STEP;
  data_output[4] = constrain(data_output[4] + cam_delta, SERVO_MIN, SERVO_MAX);

  if (in[5] == 1) {
    data_output[5] = GRIP_OPEN;
  } else if (in[5] == -1) {
    data_output[5] = GRIP_CLOSE;
  } else {
    data_output[5] = SERVO_CENTER;
  }

  for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
    data_output[i] = constrain(data_output[i], SERVO_MIN, SERVO_MAX);
  }

  ledUiMode = (int8_t)in[6];
}

#if DEBUG
static void debugPrintOutputs() {
  Serial.print(data_output[0]);
  Serial.print(' ');
  Serial.print(data_output[1]);
  Serial.print(' ');
  Serial.print(data_output[2]);
  Serial.print(' ');
  Serial.print(data_output[3]);
  Serial.print(' ');
  Serial.print(data_output[4]);
  Serial.print(' ');
  Serial.println(data_output[5]);
}
#endif

// ---------------------------------------------------------------------------
// Arduino
// ---------------------------------------------------------------------------

void setup() {
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(BITRATE);

  Serial1.setRx(UART_RX);
  Serial1.setTx(UART_TX);
  Serial1.begin(BITRATE);

  lastDataTime = millis();

  const uint16_t motor_pins[MOTOR_COUNT] = {
      PIN_MOTOR_1, PIN_MOTOR_2, PIN_MOTOR_3, PIN_MOTOR_4};
  for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
    servos[i].attach(motor_pins[i], SERVO_MIN, SERVO_MAX, 90);
    servos[i].setDirection(MOTOR_INVERT[i]);
    servos[i].setSpeed(SPEED_MOTORS);
    servos[i].setAccel(ACCEL_MOTORS);
    servos[i].setAutoDetach(false);
  }

  servos[CAM_CHANNEL_INDEX].attach(PIN_SERVO_CAM, SERVO_MIN, SERVO_MAX, 90);
  servos[CAM_CHANNEL_INDEX].setSpeed(SPEED_SERVO);
  servos[CAM_CHANNEL_INDEX].setAccel(ACCELERATE_SERVO);
  servos[CAM_CHANNEL_INDEX].writeMicroseconds(SERVO_CENTER);
  servos[CAM_CHANNEL_INDEX].setAutoDetach(true);

  servos[GRIP_CHANNEL_INDEX].attach(PIN_SERVO_ARM, SERVO_MIN, SERVO_MAX, ARM_INIT_ANGLE);
  servos[GRIP_CHANNEL_INDEX].writeMicroseconds(GRIP_CLOSE);
  servos[GRIP_CHANNEL_INDEX].setAutoDetach(false);

  delay(3000);
}

void loop() {
  // Связь: сброс fail-safe при восстановлении; иначе один раз применить дефолты.
  if (millis() - lastDataTime < DATA_TIMEOUT) {
    failSafeLatched = false;
  } else if (!failSafeLatched) {
    failSafeLatched = true;
    ledUiMode = 0;
    for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
      data_output[i] = data_output_default[i];
    }
    applyServoTargets();
  }

  if (millis() - turnTimer >= SERVO_UPDATE_INTERVAL) {
    turnTimer = millis();
    for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
      servos[i].tick();
    }
  }

  if (ledUiMode == 0) {
    if (millis() - ledTimer >= LED_BLINK_INTERVAL) {
      ledTimer = millis();
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
    }
  } else {
    digitalWrite(LED_PIN, ledUiMode > 0 ? HIGH : LOW);
  }

  if (!serialCom.available()) {
    return;
  }

  GParser data(serialCom.buf, ' ');

  if (DEBUG) {
    Serial.print(F("Received: "));
    Serial.println(serialCom.buf);
  }

  if (data.amount() != EXPECTED_DATA_COUNT) {
    return;
  }

  int data_input[INPUT_DATA_BUFFER_SIZE];
  const int parsed_count = data.parseInts(data_input);

  if (parsed_count != EXPECTED_DATA_COUNT) {
#if DEBUG
    Serial.print(F("Parse error: expected "));
    Serial.print(EXPECTED_DATA_COUNT);
    Serial.print(F(", got "));
    Serial.println(parsed_count);
#endif
    return;
  }

  if (!packetDataValid(data_input)) {
#if DEBUG
    Serial.println(F("Invalid input data range"));
#endif
    return;
  }

  lastDataTime = millis();

  fillOutputsFromPacket(data_input);

#if DEBUG
  debugPrintOutputs();
#endif

  applyServoTargets();
}
