#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include "Config.h"

// ========== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ==========
Adafruit_INA219 ina219;
bool sensor_ok = false;

// PWM измерение
volatile uint32_t pulse_width_us = 0;
volatile bool new_pulse = false;
volatile bool waiting_rising = true;
volatile uint32_t last_rising_time = 0;

// Двигатель
int16_t motor_speed = 0;
int16_t target_speed = 0;

// Защита
bool protection_active = false;
int16_t protection_dir = 0;
unsigned long motor_start_time = 0;
bool motor_running = false;
bool startup_delay = false;

// Таймеры
unsigned long last_update = 0;
unsigned long last_print = 0;
unsigned long last_led = 0;
bool led_state = false;

// ========== ПРЕРЫВАНИЕ ДЛЯ PWM ==========
void pulseInterrupt() {
    static uint32_t last_time = 0;
    uint32_t now = micros();
    
    if (now - last_time < DEBOUNCE_US) return; // Защита от дребезга
    last_time = now;
    
    bool pin_state = digitalRead(PULSE_INPUT_PIN);
    
    if (pin_state && waiting_rising) {
        last_rising_time = now;
        waiting_rising = false;
    } else if (!pin_state && !waiting_rising) {
        uint32_t width = now - last_rising_time;
        if (now < last_rising_time) {
            width = (0xFFFFFFFF - last_rising_time) + now + 1;
        }
        if (width >= PULSE_MIN_US && width <= PULSE_MAX_US) {
            pulse_width_us = width;
            new_pulse = true;
        }
        waiting_rising = true;
    }
}

// ========== ИНИЦИАЛИЗАЦИЯ ==========
void setup() {
    Serial.setRx(UART_RX);
    Serial.setTx(UART_TX);
    Serial.begin(SERIAL_BAUD_RATE);
    
    pinMode(LED_BUILTIN_PIN, OUTPUT);
    digitalWrite(LED_BUILTIN_PIN, HIGH);
    
    // PWM вход
    pinMode(PULSE_INPUT_PIN, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(PULSE_INPUT_PIN), pulseInterrupt, CHANGE);
    
    // Датчик тока
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    if (ina219.begin()) {
        sensor_ok = true;
        ina219.setCalibration_32V_1A();
    }
    
    // Драйвер двигателя
    pinMode(MOTOR_IA_PIN, OUTPUT);
    pinMode(MOTOR_IB_PIN, OUTPUT);
    analogWrite(MOTOR_IA_PIN, 0);
    analogWrite(MOTOR_IB_PIN, 0);
    
    Serial.println("=== ROV Gripper ===");
}

// ========== УПРАВЛЕНИЕ ДВИГАТЕЛЕМ ==========
void setMotor(int16_t speed) {
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

// ========== ОБРАБОТКА PWM ==========
void processPWM() {
    if (!new_pulse) return;
    
    uint32_t pulse = pulse_width_us;
    new_pulse = false;
    
    int16_t new_speed = MOTOR_SPEED_STOP;
    
    if (pulse >= PWM_MIN_US && pulse <= PWM_MAX_US) {
        if (pulse < PWM_DEADZONE_MIN_US) {
            new_speed = MOTOR_SPEED_REVERSE;
        } else if (pulse > PWM_DEADZONE_MAX_US) {
            new_speed = MOTOR_SPEED_FORWARD;
        }
    }
    
    // Сброс защиты при смене направления
    if (protection_active && new_speed != 0) {
        if ((protection_dir > 0 && new_speed < 0) || (protection_dir < 0 && new_speed > 0)) {
            protection_active = false;
        }
    }
    
    if (protection_active) {
        new_speed = MOTOR_SPEED_STOP;
    }
    
    if (new_speed != target_speed) {
        target_speed = new_speed;
        setMotor(new_speed);
        Serial.print("Motor: ");
        if (new_speed == 0) Serial.print("STOP");
        else if (new_speed > 0) Serial.print("FORWARD");
        else Serial.print("REVERSE");
        Serial.println(" (" + String(pulse) + "us)");
    }
}

// ========== ЗАЩИТА ОТ ПЕРЕГРУЗКИ ==========
void checkProtection() {
    if (!sensor_ok) return;
    
    bool motor_on = (motor_speed != 0);
    unsigned long now = millis();
    
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
            float current = ina219.getCurrent_mA();
            if (current >= CURRENT_PROTECTION_THRESHOLD_MA) {
                if (!protection_active) {
                    protection_active = true;
                    protection_dir = motor_speed;
                    Serial.println("ЗАЩИТА! Ток: " + String(current, 1) + "mA");
                }
                setMotor(0);
            }
        }
    } else {
        motor_running = false;
        startup_delay = false;
    }
}

// ========== ВЫВОД ДИАГНОСТИКИ ==========
void printDiagnostics() {
    Serial.print("Pulse: " + String(pulse_width_us) + "us | ");
    
    if (sensor_ok) {
        float current = ina219.getCurrent_mA();
        float voltage = ina219.getBusVoltage_V();
        float power = ina219.getPower_mW();
        Serial.print("I: " + String(current, 2) + "mA | ");
        Serial.print("V: " + String(voltage, 2) + "V | ");
        Serial.print("P: " + String(power, 1) + "mW | ");
    } else {
        Serial.print("Sensor: OFF | ");
    }
    
    Serial.print("Motor: " + String(motor_speed));
    
    if (protection_active) Serial.print(" [ЗАЩИТА]");
    else if (startup_delay) Serial.print(" [СТАРТ]");
    else Serial.print(" [OK]");
    
    Serial.println();
}

// ========== ОСНОВНОЙ ЦИКЛ ==========
void loop() {
    unsigned long now = millis();
    
    // Мигание LED
    if (now - last_led >= LED_BLINK_PERIOD_MS) {
        led_state = !led_state;
        digitalWrite(LED_BUILTIN_PIN, led_state ? LOW : HIGH);
        last_led = now;
    }
    
    // Основной цикл
    if (now - last_update >= MAIN_LOOP_INTERVAL_MS) {
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
}
