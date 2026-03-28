/**
 * @file config.h
 * Манипулятор ROV: пины, тайминги и константы прошивки (STM32 + Arduino framework).
 * Вход PWM от платы управления, драйвер DRV8870 (полумост IA/IB), датчик тока INA219 (I2C),
 * диагностика в UART (Serial).
 */

#pragma once

#include <Arduino.h>

// --- Пины ---
const uint16_t UART_RX = PA10;
const uint16_t UART_TX = PA9;
const uint32_t SERIAL_BAUD_RATE = 115200;

const uint16_t PULSE_INPUT_PIN = PA7;

const uint16_t I2C_SDA_PIN = PB7;
const uint16_t I2C_SCL_PIN = PB6;

// --- DRV8870 (или аналог): полумост IA/IB ---
const uint16_t MOTOR_IA_PIN = PA11;
const uint16_t MOTOR_IB_PIN = PA12;

// --- Индикация ---
const uint16_t LED_BUILTIN_PIN = PC13;
const uint16_t LED_BLINK_PERIOD_MS = 500;

// --- Декодирование PWM (мкс) ---
const uint16_t PWM_MIN_US = 900;
const uint16_t PWM_MAX_US = 2400;
const uint16_t PWM_DEADZONE_MIN_US = 1400;
const uint16_t PWM_DEADZONE_MAX_US = 1600;

/** Допустимая длина импульса для измерения (антидребезг по длительности). */
const uint16_t PULSE_MIN_US = 500;
const uint16_t PULSE_MAX_US = 3000;

const uint16_t DEBOUNCE_US = 10;

// --- Скорость двигателя (PWM 0…255 на активном полумосте) ---
const int16_t MOTOR_SPEED_FORWARD = 255;
const int16_t MOTOR_SPEED_REVERSE = -255;
const int16_t MOTOR_SPEED_STOP = 0;

// --- Защита по току ---
const float CURRENT_PROTECTION_THRESHOLD_MA = 280.0f;
const uint16_t MOTOR_START_DELAY_MS = 1000;

// --- Периоды цикла ---
const uint16_t DATA_PRINT_INTERVAL_MS = 100;
const uint16_t MAIN_LOOP_INTERVAL_MS = 20;
