#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <Arduino.h>

class PIDController {
private:
    // Параметры PID
    float kp;        // Пропорциональный коэффициент
    float ki;        // Интегральный коэффициент
    float kd;        // Дифференциальный коэффициент
    
    // Переменные состояния
    float setpoint;      // Заданное значение
    float lastInput;     // Предыдущее значение входа
    float integral;      // Интегральная составляющая
    float lastTime;      // Время последнего обновления
    
    // Ограничения
    float outputMin;     // Минимальное значение выхода
    float outputMax;     // Максимальное значение выхода
    float integralMin;   // Минимальное значение интеграла
    float integralMax;   // Максимальное значение интеграла
    
    // Настройки
    bool isEnabled;      // Включен ли контроллер
    float sampleTime;    // Время выборки в миллисекундах
    
public:
    // Конструктор
    PIDController(float p = 1.0, float i = 0.0, float d = 0.0);
    
    // Основные методы
    float calculate(float input);
    void setSetpoint(float setpoint);
    void setTunings(float p, float i, float d);
    
    // Настройка ограничений
    void setOutputLimits(float min, float max);
    void setIntegralLimits(float min, float max);
    
    // Управление
    void enable();
    void disable();
    bool isControllerEnabled() const;
    
    // Сброс состояния
    void reset();
    
    // Получение параметров
    float getKp() const;
    float getKi() const;
    float getKd() const;
    float getSetpoint() const;
    float getIntegral() const;
    
    // Настройка времени выборки
    void setSampleTime(float timeMs);
    float getSampleTime() const;
    
    // Получение статистики
    float getLastInput() const;
    float getLastOutput() const;
};

#endif // PID_CONTROLLER_H
