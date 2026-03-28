#include "BNO085.h"
#include <Wire.h>
#include <math.h>

// Global variables for SH2 library
static sh2_SensorCallback_t g_sensorCallback = nullptr;
static void *g_sensorCookie = nullptr;
static sh2_Hal_t *g_pHal = nullptr;

// Sensor event queue for processing incoming data
#define MAX_SENSOR_EVENTS 20
static sh2_SensorEvent_t sensorEventQueue[MAX_SENSOR_EVENTS];
static uint8_t eventQueueHead = 0;
static uint8_t eventQueueTail = 0;
static uint8_t eventQueueCount = 0;

// Helper functions for event queue
static bool enqueueSensorEvent(const sh2_SensorEvent_t *event) {
    if (eventQueueCount >= MAX_SENSOR_EVENTS) {
        return false; // Queue full
    }
    
    sensorEventQueue[eventQueueTail] = *event;
    eventQueueTail = (eventQueueTail + 1) % MAX_SENSOR_EVENTS;
    eventQueueCount++;
    return true;
}

static bool dequeueSensorEvent(sh2_SensorEvent_t *event) {
    if (eventQueueCount == 0) {
        return false; // Queue empty
    }
    
    *event = sensorEventQueue[eventQueueHead];
    eventQueueHead = (eventQueueHead + 1) % MAX_SENSOR_EVENTS;
    eventQueueCount--;
    return true;
}

// Constructor
BNO085::BNO085() : _i2caddr(BNO085_ADDRESS_A), _wire(&Wire), _initialized(false), _hasNewData(false) {
    _hal.context = this;
    _hal.open = BNO085::_hal_open;
    _hal.close = BNO085::_hal_close;
    _hal.read = BNO085::_hal_read;
    _hal.write = BNO085::_hal_write;
}

// Destructor
BNO085::~BNO085() {
    if (_initialized) {
        _hal_close();
    }
}

bool BNO085::begin(uint8_t addr, TwoWire *wire) {
    return begin_I2C(addr, wire);
}

bool BNO085::begin_I2C(uint8_t addr, TwoWire *wire) {
    _i2caddr = addr;
    _wire = wire;

    // Initialize I2C with proper settings for STM32F103
    _wire->begin();
    _wire->setClock(100000); // 100kHz for stability

    g_pHal = &_hal; // Set global HAL instance

    // Wait for sensor to be ready
    delay(100);

    // Initialize sensor hub with proper sequence
    if (!_initSensorHub()) {
        return false;
    }

    _initialized = true;
    return true;
}

bool BNO085::_initSensorHub() {
    // Send proper initialization sequence
    // First, send a simple command to test communication
    uint8_t testCmd[] = {0x01}; // Simple test command
    if (!_sendCommand(CHANNEL_CONTROL, testCmd, sizeof(testCmd))) {
        return false;
    }
    
    delay(50); // Wait for response
    
    // Send SHTP_START command
    uint8_t startCmd[] = {0x01, 0x00, 0x00, 0x00};
    if (!_sendCommand(CHANNEL_CONTROL, startCmd, sizeof(startCmd))) {
        return false;
    }
    
    delay(100); // Wait for sensor hub to initialize
    
    return true;
}

bool BNO085::enableReport(uint8_t sensorId, uint32_t interval_uS) {
    if (!_initialized) {
        return false;
    }
    
    // Create SET_FEATURE_COMMAND для SHTP
    uint8_t command[17];
    memset(command, 0, sizeof(command));
    
    command[0] = SHTP_REPORT_COMMAND; // 0xFD - Set Feature Command
    command[1] = sensorId;            // Feature Report ID
    command[2] = 0;                   // Feature flags
    command[3] = 0;                   // Change sensitivity (LSB)
    command[4] = 0;                   // Change sensitivity (MSB)
    
    // Report interval (little endian)
    command[5] = (uint8_t)(interval_uS & 0xFF);
    command[6] = (uint8_t)((interval_uS >> 8) & 0xFF);
    command[7] = (uint8_t)((interval_uS >> 16) & 0xFF);
    command[8] = (uint8_t)((interval_uS >> 24) & 0xFF);
    
    // Batch interval (little endian) - установить в 0 для непрерывной передачи
    command[9] = 0;
    command[10] = 0;
    command[11] = 0;
    command[12] = 0;
    
    // Sensor specific config
    command[13] = 0;
    command[14] = 0;
    command[15] = 0;
    command[16] = 0;

    // Send command on control channel (channel 2)
    bool result = _sendCommand(CHANNEL_CONTROL, command, sizeof(command));
    delay(50); // Дать время сенсору на обработку команды
    
    return result;
}

bool BNO085::disableReport(uint8_t sensorId) {
    if (!_initialized) {
        return false;
    }
    // To disable a report, set the interval to 0
    return enableReport(sensorId, 0);
}

bool BNO085::getSensorEvent(sh2_SensorValue_t *value) {
    if (!_initialized) {
        return false;
    }
    
    // Try to read data directly from I2C
    uint8_t buffer[SHTP_MAX_PAYLOAD_SIZE + SHTP_HEADER_SIZE];
    uint32_t timestamp;
    int bytes_read = _hal_read(&_hal, buffer, sizeof(buffer), &timestamp);
    
    if (bytes_read > 0) {
        // Parse the received data as a sensor event
        sh2_SensorEvent_t event;
        event.timestamp_uS = timestamp;
        event.len = bytes_read;
        
        if (bytes_read >= SHTP_HEADER_SIZE) {
            // Copy the payload into event.report
            uint8_t payload_len = min(bytes_read - SHTP_HEADER_SIZE, SH2_MAX_SENSOR_EVENT_LEN);
            memcpy(event.report, &buffer[SHTP_HEADER_SIZE], payload_len);
            event.reportId = event.report[0];
            
            // Decode the sensor event
            int rc = _decodeSensorEvent(value, &event);
            if (rc == SH2_OK) {
                _lastSensorValue = *value; // Store the last received value
                _hasNewData = true;
                return true;
            }
        }
    }
    
    return false; // No data available
}

bool BNO085::getAccelerometer(float *x, float *y, float *z) {
    sh2_SensorValue_t sensorValue;
    if (getSensorEvent(&sensorValue) && sensorValue.sensorId == SH2_ACCELEROMETER) {
        *x = sensorValue.un.accelerometer.x;
        *y = sensorValue.un.accelerometer.y;
        *z = sensorValue.un.accelerometer.z;
        return true;
    }
    return false;
}

bool BNO085::getGyroscope(float *x, float *y, float *z) {
    sh2_SensorValue_t sensorValue;
    if (getSensorEvent(&sensorValue) && sensorValue.sensorId == SH2_GYROSCOPE_CALIBRATED) {
        *x = sensorValue.un.gyroscope.x;
        *y = sensorValue.un.gyroscope.y;
        *z = sensorValue.un.gyroscope.z;
        return true;
    }
    return false;
}

bool BNO085::getMagnetometer(float *x, float *y, float *z) {
    sh2_SensorValue_t sensorValue;
    if (getSensorEvent(&sensorValue) && sensorValue.sensorId == SH2_MAGNETIC_FIELD_CALIBRATED) {
        *x = sensorValue.un.magneticField.x;
        *y = sensorValue.un.magneticField.y;
        *z = sensorValue.un.magneticField.z;
        return true;
    }
    return false;
}

bool BNO085::getRotationVector(float *i, float *j, float *k, float *real, uint16_t *accuracy) {
    sh2_SensorValue_t sensorValue;
    if (getSensorEvent(&sensorValue) && sensorValue.sensorId == SH2_ROTATION_VECTOR) {
        *i = sensorValue.un.rotationVector.i;
        *j = sensorValue.un.rotationVector.j;
        *k = sensorValue.un.rotationVector.k;
        *real = sensorValue.un.rotationVector.real;
        *accuracy = sensorValue.un.rotationVector.accuracy;
        return true;
    }
    return false;
}

bool BNO085::getEulerAngles(float *roll, float *pitch, float *yaw) {
    // Convert from quaternion to Euler angles
    float qw, qx, qy, qz;
    uint16_t accuracy;
    if (getRotationVector(&qx, &qy, &qz, &qw, &accuracy)) {
        _convertQuaternionToEuler(qw, qx, qy, qz, roll, pitch, yaw);
        return true;
    }
    return false;
}

bool BNO085::getLinearAcceleration(float *x, float *y, float *z) {
    sh2_SensorValue_t sensorValue;
    if (getSensorEvent(&sensorValue) && sensorValue.sensorId == SH2_LINEAR_ACCELERATION) {
        *x = sensorValue.un.linearAcceleration.x;
        *y = sensorValue.un.linearAcceleration.y;
        *z = sensorValue.un.linearAcceleration.z;
        return true;
    }
    return false;
}

bool BNO085::getGravity(float *x, float *y, float *z) {
    sh2_SensorValue_t sensorValue;
    if (getSensorEvent(&sensorValue) && sensorValue.sensorId == SH2_GRAVITY) {
        *x = sensorValue.un.gravity.x;
        *y = sensorValue.un.gravity.y;
        *z = sensorValue.un.gravity.z;
        return true;
    }
    return false;
}

bool BNO085::calibrateAll() {
    // Implement calibration command
    return false; 
}

bool BNO085::saveCalibration() {
    // Implement save calibration command
    return false;
}

bool BNO085::loadCalibration() {
    // Implement load calibration command
    return false;
}

bool BNO085::_sendCommand(uint8_t channel, uint8_t *data, uint8_t len) {
    uint16_t packet_len = SHTP_HEADER_SIZE + len;
    uint8_t buffer[SHTP_HEADER_SIZE + len];

    buffer[0] = (uint8_t)(packet_len & 0xFF);
    buffer[1] = (uint8_t)(packet_len >> 8);
    buffer[2] = channel;
    buffer[3] = 0; // Sequence number

    memcpy(&buffer[SHTP_HEADER_SIZE], data, len);

    return _hal_write(&_hal, buffer, packet_len) == packet_len;
}

bool BNO085::_readPacket(uint8_t *buffer, uint16_t *len, uint32_t *timestamp) {
    int bytes_read = _hal_read(&_hal, buffer, SHTP_MAX_PAYLOAD_SIZE + SHTP_HEADER_SIZE, timestamp);
    if (bytes_read > 0) {
        *len = bytes_read;
        return true;
    }
    return false;
}

bool BNO085::_waitForPacket(uint8_t channel, uint8_t *buffer, uint16_t *len, uint32_t timeout_ms) {
    unsigned long start_time = millis();
    while (millis() - start_time < timeout_ms) {
        uint32_t timestamp;
        uint16_t packet_len;
        if (_readPacket(buffer, &packet_len, &timestamp)) {
            if (buffer[2] == channel) {
                *len = packet_len;
                return true;
            }
        }
        delay(1);
    }
    return false;
}

int BNO085::_decodeSensorEvent(sh2_SensorValue_t *value, sh2_SensorEvent_t *event) {
    if (event->len < SHTP_HEADER_SIZE + 2) {
        return SH2_ERROR;
    }
    
    // Parse SH2 report header
    value->sensorId = event->report[0]; 
    value->sequence = event->report[1];
    value->status = event->report[2];

    uint8_t *data = &event->report[3]; // Data starts after header

    switch (value->sensorId) {
        case SH2_ACCELEROMETER:
            if (event->len >= SHTP_HEADER_SIZE + 3 + 12) {
                value->un.accelerometer.x = *(float*)data;
                value->un.accelerometer.y = *(float*)(data + 4);
                value->un.accelerometer.z = *(float*)(data + 8);
                return SH2_OK;
            }
            break;
            
        case SH2_GYROSCOPE_CALIBRATED:
            if (event->len >= SHTP_HEADER_SIZE + 3 + 12) {
                value->un.gyroscope.x = *(float*)data;
                value->un.gyroscope.y = *(float*)(data + 4);
                value->un.gyroscope.z = *(float*)(data + 8);
                return SH2_OK;
            }
            break;
            
        case SH2_MAGNETIC_FIELD_CALIBRATED:
            if (event->len >= SHTP_HEADER_SIZE + 3 + 12) {
                value->un.magneticField.x = *(float*)data;
                value->un.magneticField.y = *(float*)(data + 4);
                value->un.magneticField.z = *(float*)(data + 8);
                return SH2_OK;
            }
            break;
            
        case SH2_ROTATION_VECTOR:
            if (event->len >= SHTP_HEADER_SIZE + 3 + 16) {
                value->un.rotationVector.i = *(float*)data;
                value->un.rotationVector.j = *(float*)(data + 4);
                value->un.rotationVector.k = *(float*)(data + 8);
                value->un.rotationVector.real = *(float*)(data + 12);
                value->un.rotationVector.accuracy = 0;
                return SH2_OK;
            }
            break;

        case SH2_LINEAR_ACCELERATION:
            if (event->len >= SHTP_HEADER_SIZE + 3 + 12) {
                value->un.linearAcceleration.x = *(float*)data;
                value->un.linearAcceleration.y = *(float*)(data + 4);
                value->un.linearAcceleration.z = *(float*)(data + 8);
                return SH2_OK;
            }
            break;

        case SH2_GRAVITY:
            if (event->len >= SHTP_HEADER_SIZE + 3 + 12) {
                value->un.gravity.x = *(float*)data;
                value->un.gravity.y = *(float*)(data + 4);
                value->un.gravity.z = *(float*)(data + 8);
                return SH2_OK;
            }
            break;
    }
    return SH2_ERROR;
}

// Static HAL functions implementation
int BNO085::_hal_open(void) {
    return 0;
}

int BNO085::_hal_close(void) {
    return 0;
}

int BNO085::_hal_read(sh2_Hal_t *self, uint8_t *pBuffer, unsigned len, uint32_t *t_us) {
    BNO085 *bno = (BNO085*)self->context;
    TwoWire *i2c_dev = bno->_wire;
    uint8_t i2c_addr = bno->_i2caddr;

    // Try to read data from I2C
    i2c_dev->beginTransmission(i2c_addr);
    i2c_dev->write(0xFA); // Read register
    i2c_dev->endTransmission(false);

    // Read SHTP header first
    if (i2c_dev->requestFrom((uint8_t)i2c_addr, (uint8_t)4) != 4) {
        return 0; // No data available
    }

    // Read the header
    for (int i = 0; i < 4; i++) {
        pBuffer[i] = i2c_dev->read();
    }

    // Determine amount to read
    uint16_t packet_size = (uint16_t)pBuffer[0] | (uint16_t)pBuffer[1] << 8;
    packet_size &= ~0x8000; // Unset the "continue" bit

    if (packet_size == 0 || packet_size > len) {
        return 0; // Invalid packet size
    }

    // Read the rest of the packet
    if (packet_size > 4) {
        if (i2c_dev->requestFrom((uint8_t)i2c_addr, (uint8_t)(packet_size - 4)) != (packet_size - 4)) {
            return 0; // Error reading rest of packet
        }
        for (int i = 4; i < packet_size; i++) {
            pBuffer[i] = i2c_dev->read();
        }
    }
    
    *t_us = micros(); // Get current timestamp
    return packet_size;
}

int BNO085::_hal_write(sh2_Hal_t *self, uint8_t *pBuffer, unsigned int len) {
    BNO085 *bno = (BNO085*)self->context;
    TwoWire *i2c_dev = bno->_wire;
    uint8_t i2c_addr = bno->_i2caddr;

    i2c_dev->beginTransmission(i2c_addr);
    i2c_dev->write(pBuffer, len);
    return i2c_dev->endTransmission() == 0 ? len : 0;
}

// I2C specific methods
bool BNO085::_i2c_write(uint8_t *data, uint8_t len) {
    _wire->beginTransmission(_i2caddr);
    _wire->write(data, len);
    return _wire->endTransmission() == 0;
}

bool BNO085::_i2c_read(uint8_t *data, uint8_t len) {
    _wire->beginTransmission(_i2caddr);
    _wire->write(0xFA); // Read register
    _wire->endTransmission(false);

    if (_wire->requestFrom((uint8_t)_i2caddr, (uint8_t)len) != len) {
        return false;
    }
    for (int i = 0; i < len; i++) {
        data[i] = _wire->read();
    }
    return true;
}

// Utility methods
void BNO085::_convertQuaternionToEuler(float qw, float qx, float qy, float qz, 
                                       float *roll, float *pitch, float *yaw) {
    // Roll (x-axis rotation)
    float sinr_cosp = 2 * (qw * qx + qy * qz);
    float cosr_cosp = 1 - 2 * (qx * qx + qy * qy);
    *roll = atan2(sinr_cosp, cosr_cosp) * 180.0 / M_PI;

    // Pitch (y-axis rotation)
    float sinp = 2 * (qw * qy - qz * qx);
    if (fabs(sinp) >= 1)
        *pitch = copysign(M_PI / 2, sinp) * 180.0 / M_PI;
    else
        *pitch = asin(sinp) * 180.0 / M_PI;

    // Yaw (z-axis rotation)
    float siny_cosp = 2 * (qw * qz + qx * qy);
    float cosy_cosp = 1 - 2 * (qy * qy + qz * qz);
    *yaw = atan2(siny_cosp, cosy_cosp) * 180.0 / M_PI;
}

void BNO085::_convertEulerToQuaternion(float roll, float pitch, float yaw,
                                       float *qw, float *qx, float *qy, float *qz) {
    // Convert degrees to radians
    roll *= M_PI / 180.0;
    pitch *= M_PI / 180.0;
    yaw *= M_PI / 180.0;

    float cy = cos(yaw * 0.5);
    float sy = sin(yaw * 0.5);
    float cp = cos(pitch * 0.5);
    float sp = sin(pitch * 0.5);
    float cr = cos(roll * 0.5);
    float sr = sin(roll * 0.5);

    *qw = cr * cp * cy + sr * sp * sy;
    *qx = sr * cp * cy - cr * sp * sy;
    *qy = cr * sp * cy + sr * cp * sy;
    *qz = cr * cp * sy - sr * sp * cy;
}

// Global SH2 functions
void sh2_service(void) {
    // Try to read data from the sensor
    if (g_pHal && g_pHal->read) {
        uint8_t buffer[SHTP_MAX_PAYLOAD_SIZE + SHTP_HEADER_SIZE];
        uint32_t timestamp;
        int bytes_read = g_pHal->read(g_pHal, buffer, sizeof(buffer), &timestamp);
        
        if (bytes_read > 0) {
            // Parse the received data as a sensor event
            sh2_SensorEvent_t event;
            event.timestamp_uS = timestamp;
            event.len = bytes_read;
            
            if (bytes_read >= SHTP_HEADER_SIZE) {
                // Copy the payload into event.report
                memcpy(event.report, &buffer[SHTP_HEADER_SIZE], min(bytes_read - SHTP_HEADER_SIZE, SH2_MAX_SENSOR_EVENT_LEN));
                event.reportId = event.report[0];
                
                // Queue the event for processing
                enqueueSensorEvent(&event);
            }
        }
    }
    
    // Process sensor events from the queue
    sh2_SensorEvent_t event;
    while (dequeueSensorEvent(&event)) {
        if (g_sensorCallback) {
            g_sensorCallback(g_sensorCookie, &event);
        }
    }
}

int sh2_decodeSensorEvent(sh2_SensorValue_t *value, sh2_SensorEvent_t *event) {
    if (event->len < SHTP_HEADER_SIZE + 2) {
        return SH2_ERROR;
    }
    
    value->sensorId = event->report[0]; 
    value->sequence = event->report[1];
    value->status = event->report[2];

    uint8_t *data = &event->report[3];

    switch (value->sensorId) {
        case SH2_ACCELEROMETER:
            if (event->len >= SHTP_HEADER_SIZE + 3 + 12) {
                value->un.accelerometer.x = *(float*)data;
                value->un.accelerometer.y = *(float*)(data + 4);
                value->un.accelerometer.z = *(float*)(data + 8);
                return SH2_OK;
            }
            break;
            
        case SH2_GYROSCOPE_CALIBRATED:
            if (event->len >= SHTP_HEADER_SIZE + 3 + 12) {
                value->un.gyroscope.x = *(float*)data;
                value->un.gyroscope.y = *(float*)(data + 4);
                value->un.gyroscope.z = *(float*)(data + 8);
                return SH2_OK;
            }
            break;
            
        case SH2_MAGNETIC_FIELD_CALIBRATED:
            if (event->len >= SHTP_HEADER_SIZE + 3 + 12) {
                value->un.magneticField.x = *(float*)data;
                value->un.magneticField.y = *(float*)(data + 4);
                value->un.magneticField.z = *(float*)(data + 8);
                return SH2_OK;
            }
            break;
            
        case SH2_ROTATION_VECTOR:
            if (event->len >= SHTP_HEADER_SIZE + 3 + 16) {
                value->un.rotationVector.i = *(float*)data;
                value->un.rotationVector.j = *(float*)(data + 4);
                value->un.rotationVector.k = *(float*)(data + 8);
                value->un.rotationVector.real = *(float*)(data + 12);
                value->un.rotationVector.accuracy = 0;
                return SH2_OK;
            }
            break;

        case SH2_LINEAR_ACCELERATION:
            if (event->len >= SHTP_HEADER_SIZE + 3 + 12) {
                value->un.linearAcceleration.x = *(float*)data;
                value->un.linearAcceleration.y = *(float*)(data + 4);
                value->un.linearAcceleration.z = *(float*)(data + 8);
                return SH2_OK;
            }
            break;

        case SH2_GRAVITY:
            if (event->len >= SHTP_HEADER_SIZE + 3 + 12) {
                value->un.gravity.x = *(float*)data;
                value->un.gravity.y = *(float*)(data + 4);
                value->un.gravity.z = *(float*)(data + 8);
                return SH2_OK;
            }
            break;
    }
    return SH2_ERROR;
}

void sh2_setSensorCallback(sh2_SensorCallback_t callback, void *cookie) {
    g_sensorCallback = callback;
    g_sensorCookie = cookie;
}