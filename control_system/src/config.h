/**
 * @file config.h
 * Пины, тайминги и константы прошивки платы управления ROV (STM32 + Arduino framework).
 * Значения PWM для серво — микросекунды в диапазоне SERVO_MIN…SERVO_MAX.
 */

#pragma once

// --- Отладка ---
/** Включить вывод в Serial (USB); на полевых испытаниях лучше false. */
const bool DEBUG = false;

// --- Пины ---
const uint16_t LED_PIN = PC13;

const uint16_t UART_RX = PA10;
const uint16_t UART_TX = PA9;
const uint16_t BITRATE = 57600;

const uint16_t PIN_MOTOR_1 = PA6;
const uint16_t PIN_MOTOR_2 = PA5;
const uint16_t PIN_MOTOR_3 = PA4;
const uint16_t PIN_MOTOR_4 = PA3;

/** Инверсия направления приводов [M1…M4] (см. ServoSmooth::setDirection). */
const bool MOTOR_INVERT[4] = {true, false, false, false};

const uint16_t PIN_SERVO_CAM = PA7;
const uint16_t PIN_SERVO_ARM = PB0;

// --- ServoSmooth: моторы ---
const float ACCEL_MOTORS = 4.0f;
const uint16_t SPEED_MOTORS = 2500;

// --- ServoSmooth: камера ---
const uint16_t SPEED_SERVO = 70;
const float ACCELERATE_SERVO = 0.7f;

// --- Связь и безопасность ---
/** Нет валидных пакетов дольше этого времени — сброс на data_output_default, fail-safe. */
const uint32_t DATA_TIMEOUT = 1000;

// --- PWM (мкс) ---
const uint16_t SERVO_MIN = 1000;
const uint16_t SERVO_MAX = 2000;
const uint16_t SERVO_CENTER = 1500;

const uint16_t GRIP_OPEN = 2000;
const uint16_t GRIP_CLOSE = 1000;
/** Стартовый угол в градусах для attach(); далее задаётся writeMicroseconds. */
const uint16_t ARM_INIT_ANGLE = 90;

/** Число выходных каналов: M1…M4, камера, манипулятор. */
const uint8_t NUM_CHANNELS = 6;
const uint8_t MOTOR_COUNT = 4;
const uint8_t CAM_CHANNEL_INDEX = 4;
const uint8_t GRIP_CHANNEL_INDEX = 5;

/** Шаг накопления угла камеры за один тик кнопки CAM (−1/0/1). */
const int16_t CAM_STEP = 20;

const uint16_t SERVO_UPDATE_INTERVAL = 10;  // период вызова ServoSmooth::tick (мс)
const uint16_t LED_BLINK_INTERVAL = 100;      // период мигания PC13 при ledUiMode == 0 (мс)

// --- Протокол UART с геймпадом (строка, разделитель пробел, завершение \\n) ---
/** Ожидаемое число полей: joy1X joy1Y joy2X joy2Y CAM GRIP LED B7 B8 */
const uint8_t EXPECTED_DATA_COUNT = 9;
const uint8_t INPUT_DATA_BUFFER_SIZE = 10;
