/**
 * @file Config.h
 * Прошивка манипулятора: PWM с платы управления, драйвер DRV8870, датчик тока INA219 (I2C), UART.
 * Пины заданы макросами — совместимость с Arduino STM32 (PAx/PBx).
 */

#pragma once

// --- UART (телеметрия) ---
#define UART_RX PA10
#define UART_TX PA9
#define SERIAL_BAUD_RATE 115200

// --- Вход ШИМ сигнала управления ---
#define PULSE_INPUT_PIN PA7

// --- I2C (INA219) ---
#define I2C_SDA_PIN PB7
#define I2C_SCL_PIN PB6

// --- DRV8870 (или аналог): полумост IA/IB ---
#define MOTOR_IA_PIN PA11
#define MOTOR_IB_PIN PA12

// --- Индикация ---
#define LED_BUILTIN_PIN PC13
#define LED_BLINK_PERIOD_MS 500

// --- Декодирование PWM (мкс) ---
#define PWM_MIN_US 900
#define PWM_MAX_US 2400
#define PWM_DEADZONE_MIN_US 1400
#define PWM_DEADZONE_MAX_US 1600

/** Допустимая длина импульса для измерения (антидребезг по длительности). */
#define PULSE_MIN_US 500
#define PULSE_MAX_US 3000

#define DEBOUNCE_US 10

// --- Скорость двигателя (PWM 0…255 на активном полумосте) ---
#define MOTOR_SPEED_FORWARD 255
#define MOTOR_SPEED_REVERSE -255
#define MOTOR_SPEED_STOP 0

// --- Защита по току ---
#define CURRENT_PROTECTION_THRESHOLD_MA 280.0f
#define MOTOR_START_DELAY_MS 1000

// --- Периоды цикла ---
#define DATA_PRINT_INTERVAL_MS 100
#define MAIN_LOOP_INTERVAL_MS 20
