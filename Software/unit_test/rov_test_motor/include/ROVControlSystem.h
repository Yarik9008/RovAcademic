#ifndef ROV_CONTROL_SYSTEM_H
#define ROV_CONTROL_SYSTEM_H

#include <Arduino.h>
#include "IMUSensor.h"
#include "DepthSensor.h"
#include "PIDController.h"
#include "ROVMotor.h"

// Структура для команд моторов
struct MotorCommands {
    int motor0;  // Горизонтальный мотор (вперед/назад)
    int motor1;  // Горизонтальный мотор (вперед/назад)
    int motor2;  // Вертикальный мотор (вверх/вниз)
    int motor3;  // Вертикальный мотор (вверх/вниз)
    int motor4;  // Боковой мотор (влево/вправо)
    int motor5;  // Боковой мотор (влево/вправо)
    int motor6;  // Поворотный мотор
    int motor7;  // Поворотный мотор
};

// Структура для целевых значений
struct TargetValues {
    float roll;      // Целевой крен (градусы)
    float pitch;    // Целевой тангаж (градусы)
    float yaw;      // Целевое рыскание (градусы)
    float depth;    // Целевая глубина (метры)
};

// Структура для состояния системы
struct ControlSystemState {
    bool isEnabled;           // Включена ли система управления
    bool isStabilizing;       // Работает ли стабилизация
    bool sensorsReady;        // Готовы ли датчики
    unsigned long lastUpdate; // Время последнего обновления
    float updateFrequency;    // Частота обновления (Гц)
};

class ROVControlSystem {
private:
    // Датчики
    IMUSensor imu;
    DepthSensor depthSensor;
    
    // PID контроллеры
    PIDController rollPID;
    PIDController pitchPID;
    PIDController yawPID;
    PIDController depthPID;
    
    // Состояние системы
    ControlSystemState systemState;
    TargetValues targetValues;
    MotorCommands lastMotorCommands;
    
    // Настройки системы
    static const int CONTROL_FREQUENCY = 100; // Гц
    static const int CONTROL_INTERVAL = 1000 / CONTROL_FREQUENCY; // мс
    
    // Методы расчета команд моторов
    MotorCommands calculateMotorCommands();
    void distributeRollPitchCommands(float rollOutput, float pitchOutput);
    void distributeYawCommands(float yawOutput);
    void distributeDepthCommands(float depthOutput);
    
    // Методы безопасности
    void applySafetyLimits(MotorCommands& commands);
    bool validateSensorData();
    
public:
    // Конструктор
    ROVControlSystem();
    
    // Инициализация системы
    bool initialize();
    
    // Основной цикл управления
    void update();
    
    // Управление системой
    void enable();
    void disable();
    void enableStabilization();
    void disableStabilization();
    
    // Установка целевых значений
    void setTargetRoll(float roll);
    void setTargetPitch(float pitch);
    void setTargetYaw(float yaw);
    void setTargetDepth(float depth);
    void setTargetValues(const TargetValues& targets);
    
    // Настройка PID параметров
    void setRollPID(float p, float i, float d);
    void setPitchPID(float p, float i, float d);
    void setYawPID(float p, float i, float d);
    void setDepthPID(float p, float i, float d);
    
    // Получение состояния
    ControlSystemState getSystemState() const;
    TargetValues getTargetValues() const;
    MotorCommands getLastMotorCommands() const;
    
    // Получение данных датчиков
    OrientationData getOrientationData() const;
    DepthData getDepthData() const;
    
    // Получение выходов PID
    float getRollPIDOutput() const;
    float getPitchPIDOutput() const;
    float getYawPIDOutput() const;
    float getDepthPIDOutput() const;
    
    // Калибровка датчиков
    bool calibrateIMU();
    bool calibrateDepthSensor();
    
    // Экстренная остановка
    void emergencyStop();
    
    // Диагностика
    void printSystemStatus();
    void printSensorData();
    void printPIDStatus();
};

#endif // ROV_CONTROL_SYSTEM_H
