#ifndef CONFIG_H
#define CONFIG_H

// ========== КОНФИГУРАЦИЯ ПИНОВ ==========
// UART для телеметрии
#define UART_RX PA10
#define UART_TX PA9
#define SERIAL_BAUD_RATE 57600

// PWM вход для управления
#define PULSE_INPUT_PIN PA7

// I2C для датчика тока
#define I2C_SDA_PIN PB7
#define I2C_SCL_PIN PB6

// Драйвер двигателя DRV8870DDAR(MS)
#define MOTOR_IA_PIN PA11
#define MOTOR_IB_PIN PA12

// Встроенный светодиод
#define LED_BUILTIN_PIN PC13
#define LED_BLINK_PERIOD_MS 500

// ========== PWM ПАРАМЕТРЫ ==========
#define PWM_MIN_US 900
#define PWM_MAX_US 2400
#define PWM_DEADZONE_MIN_US 1400
#define PWM_DEADZONE_MAX_US 1600

// Диапазон валидных импульсов (для фильтрации)
#define PULSE_MIN_US 500
#define PULSE_MAX_US 3000

// Защита от дребезга (микросекунды)
#define DEBOUNCE_US 10

// ========== НАСТРОЙКИ ДВИГАТЕЛЯ ==========
#define MOTOR_SPEED_FORWARD 255
#define MOTOR_SPEED_REVERSE -255
#define MOTOR_SPEED_STOP 0

// ========== ЗАЩИТА ОТ ПЕРЕГРУЗКИ ==========
#define CURRENT_PROTECTION_THRESHOLD_MA 10.0
#define MOTOR_START_DELAY_MS 1000

// ========== ИНТЕРВАЛЫ И ТАЙМЕРЫ ==========

#define DATA_PRINT_INTERVAL_MS 100
#define MAIN_LOOP_INTERVAL_MS 20

#endif // CONFIG_H

