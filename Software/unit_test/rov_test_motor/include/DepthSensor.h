#ifndef DEPTH_SENSOR_H
#define DEPTH_SENSOR_H

#include <Arduino.h>
#include <Wire.h>

// Структура для хранения данных глубины
struct DepthData {
    float depth;           // Глубина в метрах
    float pressure;        // Давление в Паскалях
    float temperature;     // Температура в градусах Цельсия
    float altitude;        // Высота над уровнем моря в метрах
    bool isValid;          // Флаг корректности данных
    unsigned long timestamp; // Время получения данных
};

class DepthSensor {
private:
    int deviceAddress;
    bool isInitialized;
    DepthData currentData;
    
    // Константы для MS5611
    static const int MS5611_ADDR = 0x76;
    static const int MS5611_RESET = 0x1E;
    static const int MS5611_CONVERT_D1_256 = 0x40;
    static const int MS5611_CONVERT_D2_256 = 0x50;
    static const int MS5611_ADC_READ = 0x00;
    static const int MS5611_PROM_READ = 0xA0;
    
    // Калибровочные коэффициенты
    uint16_t C1, C2, C3, C4, C5, C6;
    
    // Константы для расчета
    static const float SEA_LEVEL_PRESSURE = 101325.0; // Па
    static const float WATER_DENSITY = 1000.0;        // кг/м³
    static const float GRAVITY = 9.80665;             // м/с²
    
    // Методы для работы с регистрами
    void writeCommand(uint8_t command);
    uint32_t readADC();
    void readCalibrationData();
    
    // Методы расчета
    float calculatePressure(uint32_t D1, uint32_t D2);
    float calculateDepth(float pressure);
    float calculateAltitude(float pressure);
    
public:
    // Конструктор
    DepthSensor(int address = MS5611_ADDR);
    
    // Инициализация датчика
    bool initialize();
    
    // Чтение данных
    bool readData();
    DepthData getCurrentData() const;
    
    // Получение отдельных компонентов
    float getDepth() const;
    float getPressure() const;
    float getTemperature() const;
    float getAltitude() const;
    
    // Проверка состояния
    bool isReady() const;
    bool isDataValid() const;
    
    // Настройка
    void setSeaLevelPressure(float pressure);
    void setWaterDensity(float density);
    
    // Калибровка на поверхности
    void calibrateSurface();
};

#endif // DEPTH_SENSOR_H
