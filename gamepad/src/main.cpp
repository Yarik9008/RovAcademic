/**
 * @file main.cpp
 * Геймпад: опрос АЦП джойстиков (Калман), кнопок, передача строки на плату управления по UART.
 * Формат Serial1: joy1X joy1Y joy2X joy2Y CAM GRIP LED B7 B8 (пробелы, завершение \\n).
 */

#include <Arduino.h>

#include "config.h"

// ---------------------------------------------------------------------------
// Калибровка и фильтры
// ---------------------------------------------------------------------------

static int joy1X_zero = ADC_MAX_VALUE / 2;
static int joy1Y_zero = ADC_MAX_VALUE / 2;
static int joy2X_zero = ADC_MAX_VALUE / 2;
static int joy2Y_zero = ADC_MAX_VALUE / 2;

static KalmanFilter kalman_joy1X = {ADC_MAX_VALUE / 2, KALMAN_P_INIT, 0, KALMAN_Q, KALMAN_R};
static KalmanFilter kalman_joy1Y = {ADC_MAX_VALUE / 2, KALMAN_P_INIT, 0, KALMAN_Q, KALMAN_R};
static KalmanFilter kalman_joy2X = {ADC_MAX_VALUE / 2, KALMAN_P_INIT, 0, KALMAN_Q, KALMAN_R};
static KalmanFilter kalman_joy2Y = {ADC_MAX_VALUE / 2, KALMAN_P_INIT, 0, KALMAN_Q, KALMAN_R};

static uint32_t ledTimer = 0;
static int ledState = LOW;

/** Период опроса и отправки пакета на Serial1. */
static unsigned long lastSendTime = 0;

// ---------------------------------------------------------------------------

static float applyKalmanFilter(KalmanFilter &kf, float measurement) {
  kf.P = kf.P + kf.Q;
  kf.K = kf.P / (kf.P + kf.R);
  kf.x = kf.x + kf.K * (measurement - kf.x);
  kf.P = (1 - kf.K) * kf.P;
  return kf.x;
}

/**
 * Чтение канала АЦП, сглаживание, мёртвая зона, масштаб в MIN…MAX_JOYSTICK_RANGE (мкс).
 */
static int readJoystick(int pin, int zero, KalmanFilter &kf, float coefficient, bool invert) {
  const int raw = analogRead(pin);
  const float filtered = applyKalmanFilter(kf, static_cast<float>(raw));
  int offset = static_cast<int>(filtered) - zero;

  if (abs(offset) < static_cast<int>(DEAD_ZONE)) {
    offset = 0;
  }

  offset = constrain(offset, -static_cast<int>(MAX_JOYSTICK_RANGE), static_cast<int>(MAX_JOYSTICK_RANGE));
  offset = static_cast<int>(offset * coefficient);

  const int range = static_cast<int>(MAX_JOYSTICK_RANGE - MIN_JOYSTIK_RANGE);
  int value = static_cast<int>(CENTER_JOYSTIK_RANGE) + (offset * range) / (MAX_JOYSTICK_RANGE * 2);

  if (invert) {
    value = 3000 - value;
  }
  return value;
}

/** Заполнить joy[0..3] и buttons[0..4] для текущего кадра. */
static void readData(int *joy, int *buttons) {
  static const JoystickCoefficients coeffs;

  joy[0] = readJoystick(JOYSTICK1_X, joy1X_zero, kalman_joy1X, coeffs.stick1_x, INVERT_JOYSTICK1_X);
  joy[1] = readJoystick(JOYSTICK1_Y, joy1Y_zero, kalman_joy1Y, coeffs.stick1_y, INVERT_JOYSTICK1_Y);
  joy[2] = readJoystick(JOYSTICK2_X, joy2X_zero, kalman_joy2X, coeffs.stick2_x, INVERT_JOYSTICK2_X);
  joy[3] = readJoystick(JOYSTICK2_Y, joy2Y_zero, kalman_joy2Y, coeffs.stick2_y, INVERT_JOYSTICK2_Y);

  if (TEST_ROBOT) {
    joy[1] = 1250;
    joy[3] = 1750;
  }

  buttons[0] = (!digitalRead(BUTTON2) && digitalRead(BUTTON1)) ? -1
               : (digitalRead(BUTTON2) && !digitalRead(BUTTON1)) ? 1
                                                                 : 0;
  buttons[1] = (!digitalRead(BUTTON3) && digitalRead(BUTTON4)) ? -1
               : (digitalRead(BUTTON3) && !digitalRead(BUTTON4)) ? 1
                                                                 : 0;
  buttons[2] = (!digitalRead(BUTTON5) && digitalRead(BUTTON6)) ? 1
               : (digitalRead(BUTTON5) && !digitalRead(BUTTON6)) ? -1
                                                                 : 0;
  buttons[3] = !digitalRead(BUTTON7) ? 1 : 0;
  buttons[4] = !digitalRead(BUTTON8) ? 1 : 0;
}

#if DEBUG
static void debugPrintFrame(const int *joy, const int *buttons) {
  Serial.print(F("joy1X:"));
  Serial.print(joy[0]);
  Serial.print(F(" joy1Y:"));
  Serial.print(joy[1]);
  Serial.print(F(" joy2X:"));
  Serial.print(joy[2]);
  Serial.print(F(" joy2Y:"));
  Serial.print(joy[3]);
  Serial.print(F(" CAM:"));
  Serial.print(buttons[0]);
  Serial.print(F(" GRIP:"));
  Serial.print(buttons[1]);
  Serial.print(F(" LED:"));
  Serial.print(buttons[2]);
  Serial.print(F(" B7:"));
  Serial.print(buttons[3]);
  Serial.print(F(" B8:"));
  Serial.println(buttons[4]);
}
#endif

static void sendFrameToControlBoard(const int *joy, const int *buttons) {
  Serial1.print(joy[0]);
  Serial1.print(' ');
  Serial1.print(joy[1]);
  Serial1.print(' ');
  Serial1.print(joy[2]);
  Serial1.print(' ');
  Serial1.print(joy[3]);
  Serial1.print(' ');
  Serial1.print(buttons[0]);
  Serial1.print(' ');
  Serial1.print(buttons[1]);
  Serial1.print(' ');
  Serial1.print(buttons[2]);
  Serial1.print(' ');
  Serial1.print(buttons[3]);
  Serial1.print(' ');
  Serial1.println(buttons[4]);
}

// ---------------------------------------------------------------------------

void setup() {
  pinMode(LED_PIN, OUTPUT);

  const uint16_t buttonPins[] = {
      BUTTON1, BUTTON2, BUTTON3, BUTTON4, BUTTON5, BUTTON6, BUTTON7, BUTTON8};
  for (uint8_t i = 0; i < 8; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  analogReadResolution(12);

  joy1X_zero = analogRead(JOYSTICK1_X);
  joy1Y_zero = analogRead(JOYSTICK1_Y);
  joy2X_zero = analogRead(JOYSTICK2_X);
  joy2Y_zero = analogRead(JOYSTICK2_Y);

  kalman_joy1X.x = static_cast<float>(joy1X_zero);
  kalman_joy1Y.x = static_cast<float>(joy1Y_zero);
  kalman_joy2X.x = static_cast<float>(joy2X_zero);
  kalman_joy2Y.x = static_cast<float>(joy2Y_zero);

  Serial.begin(BITRATE);
  Serial1.setRx(UART_RX);
  Serial1.setTx(UART_TX);
  Serial1.begin(BITRATE);
}

void loop() {
  const unsigned long now = millis();

  if (millis() - ledTimer >= LED_BLINK_INTERVAL_MS) {
    ledTimer = millis();
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }

  if (now - lastSendTime < DATA_INTERVAL) {
    return;
  }

  int joy[JOY_AXIS_COUNT];
  int buttons[BUTTON_FIELD_COUNT];

  readData(joy, buttons);

#if DEBUG
  debugPrintFrame(joy, buttons);
#endif

  sendFrameToControlBoard(joy, buttons);

  lastSendTime = now;
}
