#ifndef ROVMOTOR_H
#define ROVMOTOR_H

#include <Arduino.h>
#include <Servo.h>

class ROVMotor {
private:
    Servo esc;                          // ESC для управления BLDC мотором
    int escPin;                         // Пин подключения ESC
    int currentSpeed;                  // Текущая скорость мотора
    int targetSpeed;                    // Целевая скорость мотора
    int speedChange;                    // Изменение скорости за цикл
    unsigned long lastUpdateTime;       // Время последнего обновления
    unsigned long updateInterval;       // Интервал обновления (10мс = 100Гц)
    
    // Константы для BLDC мотора
    static const int MOTOR_STOP = 1500; // Остановка мотора
    static const int MOTOR_MIN = 1000;  // Минимальная скорость (реверс)
    static const int MOTOR_MAX = 2000;  // Максимальная скорость (вперед)
    
    // Статические переменные для управления массивом моторов
    static const int MAX_MOTORS = 8;   // Максимальное количество моторов
    static ROVMotor* motors[MAX_MOTORS]; // Массив указателей на моторы
    static int motorCount;              // Количество активных моторов
    
public:
    // Конструктор
    ROVMotor(int pin);
    
    // Деструктор
    ~ROVMotor();
    
    // Инициализация мотора (аналог attach из Servo)
    void begin();
    void begin(int min, int max);
    
    // Основные методы управления (в стиле ServoSmooth)
    bool tick();                        // Обновление мотора с встроенным таймером
    void setTarget(int target);         // Установка целевой скорости в микросекундах
    void setTargetPercent(int percent); // Установка скорости в процентах (-100 до +100)
    void setSpeed(int speed);           // Установка максимальной скорости изменения
    void stop();                        // Остановка мотора
    
    // Получение состояния
    int getCurrent() const;             // Текущая скорость в микросекундах
    int getTarget() const;              // Целевая скорость в микросекундах
    int getCurrentPercent() const;      // Текущая скорость в процентах
    int getTargetPercent() const;       // Целевая скорость в процентах
    bool isTargetReached() const;       // Достигнута ли целевая скорость
    
    // Получение констант
    static int getMotorStop() { return MOTOR_STOP; }
    static int getMotorMin() { return MOTOR_MIN; }
    static int getMotorMax() { return MOTOR_MAX; }
    
    // Статические методы для управления массивом моторов
    static bool addMotor(int pin);
    static void beginAll();
    static bool setMotorSpeed(int motorIndex, int speed);
    static void setAllMotorsSpeed(int speed);
    static bool setMotorPercent(int motorIndex, int percent);
    static void setAllMotorsPercent(int percent);
    static bool stopMotor(int motorIndex);
    static void stopAllMotors();
    static bool tickAll();              // Обновление всех моторов с таймером
    static int getMotorCount() { return motorCount; }
    static int getMotorCurrentSpeed(int motorIndex);
    static int getMotorTargetSpeed(int motorIndex);
    static bool isMotorTargetReached(int motorIndex);
};

#endif // ROVMOTOR_H
