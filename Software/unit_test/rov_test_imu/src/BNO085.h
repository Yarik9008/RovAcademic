#ifndef BNO085_H
#define BNO085_H

#include <Arduino.h>
#include <Wire.h>

// Константы для BNO085
#define BNO085_ADDRESS_A 0x4A
#define BNO085_ADDRESS_B 0x4B

// SHTP (Sensor Hub Transport Protocol) константы
#define SHTP_HEADER_SIZE 4
#define SHTP_MAX_PAYLOAD_SIZE 512
#define SHTP_MAX_SENSOR_EVENT_LEN 16
#define SH2_MAX_SENSOR_EVENT_LEN SHTP_MAX_SENSOR_EVENT_LEN

// SHTP Channel Numbers
#define CHANNEL_CONTROL 0
#define CHANNEL_EXECUTABLE 1
#define CHANNEL_SENSOR_REPORT 2
#define CHANNEL_WAKE_REPORT 3

// SHTP Commands
#define SHTP_REPORT_COMMAND 0xFD
#define SHTP_REQUEST_REPORT_COMMAND 0xFC
#define SHTP_GET_FEATURE_REQUEST_COMMAND 0xFE
#define SHTP_CLEAR_DTU_COMMAND 0xFF

// Sensor Report IDs (SH2)
#define SH2_ACCELEROMETER 0x01
#define SH2_GYROSCOPE_CALIBRATED 0x02
#define SH2_MAGNETIC_FIELD_CALIBRATED 0x03
#define SH2_ROTATION_VECTOR 0x05
#define SH2_LINEAR_ACCELERATION 0x06
#define SH2_GRAVITY 0x07
#define SH2_GAME_ROTATION_VECTOR 0x08
#define SH2_GEOMAGNETIC_ROTATION_VECTOR 0x09

// Sensor Event Status
#define SH2_OK 0
#define SH2_ERROR -1

// Структуры данных
typedef struct {
    uint32_t timestamp_uS;
    uint8_t reportId;
    uint8_t len;
    uint8_t report[SH2_MAX_SENSOR_EVENT_LEN];
} sh2_SensorEvent_t;

typedef struct {
    uint8_t sensorId;
    uint8_t sequence;
    uint8_t status;
    union {
        struct {
            float x, y, z;
            uint8_t status;
        } accelerometer;
        struct {
            float x, y, z;
            uint8_t status;
        } gyroscope;
        struct {
            float x, y, z;
            uint8_t status;
        } magneticField;
        struct {
            float i, j, k, real;
            uint16_t accuracy;
        } rotationVector;
        struct {
            float x, y, z;
            uint8_t status;
        } linearAcceleration;
        struct {
            float x, y, z;
            uint8_t status;
        } gravity;
        struct {
            float roll, pitch, yaw;
            uint8_t status;
        } eulerAngles;
    } un;
} sh2_SensorValue_t;

// HAL (Hardware Abstraction Layer) структура
typedef struct sh2_Hal_t {
    int (*open)(void);
    int (*close)(void);
    int (*read)(struct sh2_Hal_t *self, uint8_t *pBuffer, unsigned len, uint32_t *t_us);
    int (*write)(struct sh2_Hal_t *self, uint8_t *pBuffer, unsigned len);
    void *context;
} sh2_Hal_t;

// Callback функция для обработки сенсорных событий
typedef void (*sh2_SensorCallback_t)(void *cookie, sh2_SensorEvent_t *event);

// Основной класс BNO085
class BNO085 {
public:
    BNO085();
    ~BNO085();
    
    // Инициализация
    bool begin(uint8_t addr = BNO085_ADDRESS_A, TwoWire *wire = &Wire);
    bool begin_I2C(uint8_t addr = BNO085_ADDRESS_A, TwoWire *wire = &Wire);
    
    // Управление отчетами сенсоров
    bool enableReport(uint8_t sensorId, uint32_t interval_uS);
    bool disableReport(uint8_t sensorId);
    bool getSensorEvent(sh2_SensorValue_t *value);

    // Получение конкретных данных сенсоров
    bool getAccelerometer(float *x, float *y, float *z);
    bool getGyroscope(float *x, float *y, float *z);
    bool getMagnetometer(float *x, float *y, float *z);
    bool getRotationVector(float *i, float *j, float *k, float *real, uint16_t *accuracy);
    bool getEulerAngles(float *roll, float *pitch, float *yaw);
    bool getLinearAcceleration(float *x, float *y, float *z);
    bool getGravity(float *x, float *y, float *z);
    
    // Калибровка
    bool calibrateAll();
    bool saveCalibration();
    bool loadCalibration();
    
    // Статические методы для работы с SH2
    static void sh2_service(void);
    static int sh2_decodeSensorEvent(sh2_SensorValue_t *value, sh2_SensorEvent_t *event);
    static void sh2_setSensorCallback(sh2_SensorCallback_t callback, void *cookie);

private:
    TwoWire *_wire;
    uint8_t _i2caddr;
    sh2_Hal_t _hal;
    bool _initialized;
    
    // Буферы для данных
    sh2_SensorValue_t _lastSensorValue;
    bool _hasNewData;
    
    // Private methods for SH2 interaction
    bool _initSensorHub();
    int _decodeSensorEvent(sh2_SensorValue_t *value, sh2_SensorEvent_t *event);
    bool _sendCommand(uint8_t channel, uint8_t *data, uint8_t len);
    bool _readPacket(uint8_t *buffer, uint16_t *len, uint32_t *timestamp);
    bool _waitForPacket(uint8_t channel, uint8_t *buffer, uint16_t *len, uint32_t timeout_ms);
    
    // HAL functions
    static int _hal_open(void);
    static int _hal_close(void);
    static int _hal_read(sh2_Hal_t *self, uint8_t *pBuffer, unsigned len, uint32_t *t_us);
    static int _hal_write(sh2_Hal_t *self, uint8_t *pBuffer, unsigned int len);
    
    // I2C specific methods
    bool _i2c_write(uint8_t *data, uint8_t len);
    bool _i2c_read(uint8_t *data, uint8_t len);
    
    // Utility methods
    void _convertQuaternionToEuler(float qw, float qx, float qy, float qz, 
                                   float *roll, float *pitch, float *yaw);
    void _convertEulerToQuaternion(float roll, float pitch, float yaw,
                                   float *qw, float *qx, float *qy, float *qz);
};

#endif // BNO085_H