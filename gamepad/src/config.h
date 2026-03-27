/**
 * @file config.h
 * Пульт управления ROV: пины джойстиков и кнопок, нормализация в PWM (мкс), фильтр Калмана, UART к плате управления.
 */

#pragma once

// --- Отладка и тесты ---
/** Вывод осей и кнопок в Serial (USB). */
const bool DEBUG = false;

/** Подмена осей для стендового теста (см. readData). */
const bool TEST_ROBOT = false;

// --- Пины ---
const uint16_t LED_PIN = PC13;

const uint16_t UART_RX = PA10;
const uint16_t UART_TX = PA9;
const uint16_t BITRATE = 57600;

const uint16_t JOYSTICK1_X = PA3;
const uint16_t JOYSTICK1_Y = PA4;
const uint16_t JOYSTICK2_X = PA5;
const uint16_t JOYSTICK2_Y = PA6;

const bool INVERT_JOYSTICK1_X = false;
const bool INVERT_JOYSTICK1_Y = true;
const bool INVERT_JOYSTICK2_X = true;
const bool INVERT_JOYSTICK2_Y = false;

const uint16_t BUTTON1 = PA15;
const uint16_t BUTTON2 = PB3;
const uint16_t BUTTON3 = PB4;
const uint16_t BUTTON4 = PB5;
const uint16_t BUTTON5 = PB6;
const uint16_t BUTTON6 = PB7;
const uint16_t BUTTON7 = PB8;
const uint16_t BUTTON8 = PB9;

/** Число осей (joy1X…joy2Y), выход в мкс. */
const uint8_t JOY_AXIS_COUNT = 4;

/**
 * Поля кнопок в пакете на Serial1: CAM, GRIP, LED, B7, B8.
 * Значения трёхпозиционных: −1, 0, +1; B7/B8: 0/1.
 */
const uint8_t BUTTON_FIELD_COUNT = 5;

// --- ADC (12 бит на STM32) ---
const uint16_t ADC_MAX_VALUE = 4095;

const uint16_t DEAD_ZONE = 250;

/** Имена с опечаткой (JOYSTIK) сохранены — используются в readJoystick. */
const uint16_t MIN_JOYSTIK_RANGE = 1000;
const uint16_t CENTER_JOYSTIK_RANGE = 1500;
const uint16_t MAX_JOYSTICK_RANGE = 2000;

// --- Фильтр Калмана (сырое ADC → сглаженное) ---
const float KALMAN_Q = 25.0f;
const float KALMAN_R = 20.0f;
const float KALMAN_P_INIT = 1000.0f;

// --- Периоды (мс) ---
const uint16_t DATA_INTERVAL = 20;
/** Мигание LED_PIN на пульте. */
const uint16_t LED_BLINK_INTERVAL_MS = 500;

struct JoystickCoefficients {
  float stick1_x = 0.0f;
  float stick1_y = 0.7f;
  float stick2_x = 0.5f;
  float stick2_y = 0.7f;
};

struct KalmanFilter {
  float x;
  float P;
  float K;
  float Q;
  float R;
};
