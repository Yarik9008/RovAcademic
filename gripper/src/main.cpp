/**
 * @file main.cpp
 * Манипулятор: измерение PWM с платы управления, приведение к направлению/остановке мотора,
 * защита по току INA219, телеметрия в Serial.
 */

#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_INA219.h>

#include "config.h"

// ---------------------------------------------------------------------------
// Глобальное состояние
// ---------------------------------------------------------------------------

static Adafruit_INA219 ina219;
static bool sensor_ok = false;

static volatile uint32_t pulse_width_us = 0;
static volatile bool new_pulse = false;
static volatile bool waiting_rising = true;
static volatile uint32_t last_rising_time = 0;

static int16_t motor_speed = 0;
static int16_t target_speed = 0;

static bool protection_active = false;
static int16_t protection_dir = 0;
static unsigned long motor_start_time = 0;
static bool motor_running = false;
static bool startup_delay = false;

static unsigned long last_update = 0;
static unsigned long last_print = 0;
static unsigned long last_led = 0;
static bool led_state = false;

// ---------------------------------------------------------------------------

void pulseInterrupt() {
  static uint32_t last_time = 0;
  const uint32_t now = micros();

  if (now - last_time < DEBOUNCE_US) {
    return;
  }
  last_time = now;

  const bool pin_state = digitalRead(PULSE_INPUT_PIN);

  if (pin_state && waiting_rising) {
    last_rising_time = now;
    waiting_rising = false;
  } else if (!pin_state && !waiting_rising) {
    uint32_t width = now - last_rising_time;
    if (now < last_rising_time) {
      width = (0xFFFFFFFFu - last_rising_time) + now + 1u;
    }
    if (width >= PULSE_MIN_US && width <= PULSE_MAX_US) {
      pulse_width_us = width;
      new_pulse = true;
    }
    waiting_rising = true;
  }
}

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
  motor_speed = speed;
}

static void processPWM() {
  if (!new_pulse) {
    return;
  }

  const uint32_t pulse = pulse_width_us;
  new_pulse = false;

  int16_t new_speed = MOTOR_SPEED_STOP;

  if (pulse >= PWM_MIN_US && pulse <= PWM_MAX_US) {
    if (pulse < PWM_DEADZONE_MIN_US) {
      new_speed = MOTOR_SPEED_REVERSE;
    } else if (pulse > PWM_DEADZONE_MAX_US) {
      new_speed = MOTOR_SPEED_FORWARD;
    }
  }

  if (protection_active && new_speed != 0) {
    if ((protection_dir > 0 && new_speed < 0) || (protection_dir < 0 && new_speed > 0)) {
      protection_active = false;
    }
  }

  if (protection_active) {
    new_speed = MOTOR_SPEED_STOP;
  }

  if (new_speed == target_speed) {
    return;
  }

  target_speed = new_speed;
  setMotor(new_speed);

  Serial.print(F("Motor: "));
  if (new_speed == 0) {
    Serial.print(F("STOP"));
  } else if (new_speed > 0) {
    Serial.print(F("FORWARD"));
  } else {
    Serial.print(F("REVERSE"));
  }
  Serial.print(F(" ("));
  Serial.print(pulse);
  Serial.println(F("us)"));
}

static void checkProtection() {
  if (!sensor_ok) {
    return;
  }

  const bool motor_on = (motor_speed != 0);
  const unsigned long now = millis();

  if (motor_on) {
    if (!motor_running) {
      motor_start_time = now;
      motor_running = true;
      startup_delay = true;
    }

    if (startup_delay && (now - motor_start_time >= MOTOR_START_DELAY_MS)) {
      startup_delay = false;
    }

    if (!startup_delay) {
      const float current = ina219.getCurrent_mA();
      if (current >= CURRENT_PROTECTION_THRESHOLD_MA) {
        if (!protection_active) {
          protection_active = true;
          protection_dir = motor_speed;
          Serial.print(F("ЗАЩИТА! Ток: "));
          Serial.print(current, 1);
          Serial.println(F(" mA"));
        }
        setMotor(0);
      }
    }
  } else {
    motor_running = false;
    startup_delay = false;
  }
}

static void printDiagnostics() {
  Serial.print(F("Pulse: "));
  Serial.print(pulse_width_us);
  Serial.print(F("us | "));

  if (sensor_ok) {
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
  Serial.print(motor_speed);

  if (protection_active) {
    Serial.print(F(" [ЗАЩИТА]"));
  } else if (startup_delay) {
    Serial.print(F(" [СТАРТ]"));
  } else {
    Serial.print(F(" [OK]"));
  }

  Serial.println();
}

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
    sensor_ok = true;
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

  if (now - last_led >= LED_BLINK_PERIOD_MS) {
    led_state = !led_state;
    digitalWrite(LED_BUILTIN_PIN, led_state ? LOW : HIGH);
    last_led = now;
  }

  if (now - last_update < MAIN_LOOP_INTERVAL_MS) {
    return;
  }

  checkProtection();

  if (new_pulse) {
    processPWM();
  }

  if (now - last_print >= DATA_PRINT_INTERVAL_MS) {
    printDiagnostics();
    last_print = now;
  }

  last_update = now;
}
