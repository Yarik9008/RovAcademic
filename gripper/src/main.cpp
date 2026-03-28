/**
 * @file main.cpp
 * Манипулятор ROV: приём PWM-команды от платы управления, управление мотором (DRV8870),
 * защита по току через INA219 (I2C), диагностика в Serial (UART).
 *
 * Цикл: декодирование PWM → определение направления → контроль тока → телеметрия.
 */

#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_INA219.h>

#include "config.h"

// ---------------------------------------------------------------------------
// Глобальное состояние
// ---------------------------------------------------------------------------

static Adafruit_INA219 ina219;
static bool sensorOk = false;

static volatile uint32_t pulseWidthUs = 0;
static volatile bool newPulse = false;
static volatile bool waitingRising = true;
static volatile uint32_t lastRisingTime = 0;

static int16_t motorSpeed = 0;
static int16_t targetSpeed = 0;

static bool protectionActive = false;
static int16_t protectionDir = 0;
static unsigned long motorStartTime = 0;
static bool motorRunning = false;
static bool startupDelay = false;

static unsigned long lastUpdate = 0;
static unsigned long lastPrint = 0;
static unsigned long lastLed = 0;
static bool ledState = false;

// ---------------------------------------------------------------------------
// Вспомогательные функции
// ---------------------------------------------------------------------------

/** ISR: измерение ширины импульса на PULSE_INPUT_PIN (CHANGE), антидребезг DEBOUNCE_US. */
void pulseInterrupt() {
  static uint32_t lastTime = 0;
  const uint32_t now = micros();

  if (now - lastTime < DEBOUNCE_US) {
    return;
  }
  lastTime = now;

  const bool pinState = digitalRead(PULSE_INPUT_PIN);

  if (pinState && waitingRising) {
    lastRisingTime = now;
    waitingRising = false;
  } else if (!pinState && !waitingRising) {
    uint32_t width = now - lastRisingTime;
    if (now < lastRisingTime) {
      width = (0xFFFFFFFFu - lastRisingTime) + now + 1u;
    }
    if (width >= PULSE_MIN_US && width <= PULSE_MAX_US) {
      pulseWidthUs = width;
      newPulse = true;
    }
    waitingRising = true;
  }
}

/** Установить скорость мотора через полумост DRV8870 (0 — стоп, >0 — вперёд, <0 — назад). */
static void setMotor(int16_t speed) {
  if (speed == 0) {
    analogWrite(MOTOR_IA_PIN, 0);
    analogWrite(MOTOR_IB_PIN, 0);
  } else if (speed > 0) {
    analogWrite(MOTOR_IA_PIN, speed);
    analogWrite(MOTOR_IB_PIN, 0);
  } else {
    analogWrite(MOTOR_IA_PIN, 0);
    analogWrite(MOTOR_IB_PIN, speed);
  }
  motorSpeed = speed;
}

/** Декодирование ширины импульса в команду мотора; учитывает токовую защиту. */
static void processPWM() {
  if (!newPulse) {
    return;
  }

  const uint32_t pulse = pulseWidthUs;
  newPulse = false;

  int16_t newSpeed = MOTOR_SPEED_STOP;

  if (pulse >= PWM_MIN_US && pulse <= PWM_MAX_US) {
    if (pulse < PWM_DEADZONE_MIN_US) {
      newSpeed = MOTOR_SPEED_REVERSE;
    } else if (pulse > PWM_DEADZONE_MAX_US) {
      newSpeed = MOTOR_SPEED_FORWARD;
    }
  }

  if (protectionActive && newSpeed != 0) {
    if ((protectionDir > 0 && newSpeed < 0) || (protectionDir < 0 && newSpeed > 0)) {
      protectionActive = false;
    }
  }

  if (protectionActive) {
    newSpeed = MOTOR_SPEED_STOP;
  }

  if (newSpeed == targetSpeed) {
    return;
  }

  targetSpeed = newSpeed;
  setMotor(newSpeed);

  Serial.print(F("Motor: "));
  if (newSpeed == 0) {
    Serial.print(F("STOP"));
  } else if (newSpeed > 0) {
    Serial.print(F("FORWARD"));
  } else {
    Serial.print(F("REVERSE"));
  }
  Serial.print(F(" ("));
  Serial.print(pulse);
  Serial.println(F("us)"));
}

/** Опрос INA219; при превышении CURRENT_PROTECTION_THRESHOLD_MA — аварийная остановка. */
static void checkProtection() {
  if (!sensorOk) {
    return;
  }

  const bool motorOn = (motorSpeed != 0);
  const unsigned long now = millis();

  if (motorOn) {
    if (!motorRunning) {
      motorStartTime = now;
      motorRunning = true;
      startupDelay = true;
    }

    if (startupDelay && (now - motorStartTime >= MOTOR_START_DELAY_MS)) {
      startupDelay = false;
    }

    if (!startupDelay) {
      const float current = ina219.getCurrent_mA();
      if (current >= CURRENT_PROTECTION_THRESHOLD_MA) {
        if (!protectionActive) {
          protectionActive = true;
          protectionDir = motorSpeed;
          Serial.print(F("ЗАЩИТА! Ток: "));
          Serial.print(current, 1);
          Serial.println(F(" mA"));
        }
        setMotor(0);
      }
    }
  } else {
    motorRunning = false;
    startupDelay = false;
  }
}

/** Вывод телеметрии в Serial: ширина импульса, ток/напряжение/мощность, состояние мотора. */
static void printDiagnostics() {
  Serial.print(F("Pulse: "));
  Serial.print(pulseWidthUs);
  Serial.print(F("us | "));

  if (sensorOk) {
    const float current = ina219.getCurrent_mA();
    const float voltage = ina219.getBusVoltage_V();
    const float power = ina219.getPower_mW();
    Serial.print(F("I: "));
    Serial.print(current, 2);
    Serial.print(F("mA | V: "));
    Serial.print(voltage, 2);
    Serial.print(F("V | P: "));
    Serial.print(power, 1);
    Serial.print(F("mW | "));
  } else {
    Serial.print(F("Sensor: OFF | "));
  }

  Serial.print(F("Motor: "));
  Serial.print(motorSpeed);

  if (protectionActive) {
    Serial.print(F(" [ЗАЩИТА]"));
  } else if (startupDelay) {
    Serial.print(F(" [СТАРТ]"));
  } else {
    Serial.print(F(" [OK]"));
  }

  Serial.println();
}

// ---------------------------------------------------------------------------
// Arduino
// ---------------------------------------------------------------------------

void setup() {
  Serial.setRx(UART_RX);
  Serial.setTx(UART_TX);
  Serial.begin(SERIAL_BAUD_RATE);

  pinMode(LED_BUILTIN_PIN, OUTPUT);
  digitalWrite(LED_BUILTIN_PIN, HIGH);

  pinMode(PULSE_INPUT_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(PULSE_INPUT_PIN), pulseInterrupt, CHANGE);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  if (ina219.begin()) {
    sensorOk = true;
    ina219.setCalibration_32V_1A();
  }

  pinMode(MOTOR_IA_PIN, OUTPUT);
  pinMode(MOTOR_IB_PIN, OUTPUT);
  analogWrite(MOTOR_IA_PIN, 0);
  analogWrite(MOTOR_IB_PIN, 0);

  Serial.println(F("=== ROV Gripper ==="));
}

void loop() {
  const unsigned long now = millis();

  if (now - lastLed >= LED_BLINK_PERIOD_MS) {
    ledState = !ledState;
    digitalWrite(LED_BUILTIN_PIN, ledState ? LOW : HIGH);
    lastLed = now;
  }

  if (now - lastUpdate < MAIN_LOOP_INTERVAL_MS) {
    return;
  }

  checkProtection();

  if (newPulse) {
    processPWM();
  }

  if (now - lastPrint >= DATA_PRINT_INTERVAL_MS) {
    printDiagnostics();
    lastPrint = now;
  }

  lastUpdate = now;
}
