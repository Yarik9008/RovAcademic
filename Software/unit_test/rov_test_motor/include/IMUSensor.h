#ifndef IMU_SENSOR_H
#define IMU_SENSOR_H

#include <Arduino.h>
#include <Wire.h>

// Структура для хранения данных ориентации
struct OrientationData {
    float roll;      // Крен (градусы)
    float pitch;     // Тангаж (градусы)
    float yaw;       // Рыскание (градусы)
    float rollRate;  // Угловая скорость крена (град/с)
    float pitchRate; // Угловая скорость тангажа (град/с)
    float yawRate;   // Угловая скорость рыскания (град/с)
    bool isValid;    // Флаг корректности данных
    unsigned long timestamp; // Время получения данных
};

class IMUSensor {
private:
    int deviceAddress;
    bool isInitialized;
    OrientationData currentData;
    OrientationData calibrationOffset;
    
    // Константы для MPU6050
    static const int MPU6050_ADDR = 0x68;
    static const int MPU6050_PWR_MGMT_1 = 0x6B;
    static const int MPU6050_ACCEL_XOUT_H = 0x3B;
    static const int MPU6050_GYRO_XOUT_H = 0x43;
    
    // Методы для работы с регистрами
    void writeRegister(uint8_t reg, uint8_t value);
    uint8_t readRegister(uint8_t reg);
    void readRegisters(uint8_t reg, uint8_t* buffer, uint8_t length);
    
    // Методы обработки данных
    float processAccelerometerData(int16_t rawData);
    float processGyroscopeData(int16_t rawData);
    void calculateAngles();
    
public:
    // Конструктор
    IMUSensor(int address = MPU6050_ADDR);
    
    // Инициализация датчика
    bool initialize();
    
    // Калибровка датчика
    bool calibrate(int samples = 1000);
    
    // Чтение данных
    bool readData();
    OrientationData getCurrentData() const;
    
    // Получение отдельных компонентов
    float getRoll() const;
    float getPitch() const;
    float getYaw() const;
    float getRollRate() const;
    float getPitchRate() const;
    float getYawRate() const;
    
    // Проверка состояния
    bool isReady() const;
    bool isDataValid() const;
    
    // Настройка фильтрации
    void setFilterAlpha(float alpha); // 0.0 - 1.0, где 1.0 = без фильтрации
};

#endif // IMU_SENSOR_H
