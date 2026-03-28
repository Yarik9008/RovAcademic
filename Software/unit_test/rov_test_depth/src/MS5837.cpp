#include "MS5837.h"
#include <Wire.h>

// I2C constants
const uint8_t MS5837_ADDR = 0x76;
const uint8_t MS5837_RESET = 0x1E;
const uint8_t MS5837_ADC_READ = 0x00;
const uint8_t MS5837_PROM_READ = 0xA0;
const uint8_t MS5837_CONVERT_D1_8192 = 0x4A;
const uint8_t MS5837_CONVERT_D2_8192 = 0x5A;

// Model detection constants
const uint16_t MS5837_02BA_MAX_SENSITIVITY = 49000;
const uint16_t MS5837_02BA_30BA_SEPARATION = 37000;
const uint16_t MS5837_30BA_MIN_SENSITIVITY = 26000;

// Timeouts (milliseconds)
const unsigned long RESET_TIMEOUT = 10;
const unsigned long CONVERSION_TIMEOUT = 25;
const unsigned long PROM_READ_TIMEOUT = 5;

MS5837::MS5837() {
	fluidDensity = 1029; // Default: seawater
	surfacePressure = 0.0;
	surfaceCalibrated = false;
	_state = IDLE;
	_operationStartTime = 0;
	_promIndex = 0;
	_initComplete = false;
}

bool MS5837::init(TwoWire &wirePort) {
	_i2cPort = &wirePort;
	_state = INIT_RESET;
	_operationStartTime = millis();
	_promIndex = 0;
	_initComplete = false;
	
	initReset();
	return true; // Always return true - initialization started
}

void MS5837::setFluidDensity(float density) {
	fluidDensity = density;
}

void MS5837::calibrateSurface() {
	surfacePressure = pressure() * 100.0f; // Convert mbar to Pa
	surfaceCalibrated = true;
}

bool MS5837::isReady() {
	return (_state == IDLE && _initComplete);
}

void MS5837::continueInit() {
	if (_initComplete) return;
	
	switch (_state) {
		case INIT_RESET:
			initReset();
			break;
		case INIT_READ_PROM:
			initReadPROM();
			break;
		default:
			break;
	}
}

bool MS5837::read() {
	if (_i2cPort == NULL || !_initComplete) {
		return false;
	}

	switch (_state) {
		case IDLE:
			_state = READ_REQUEST_D1;
			_operationStartTime = millis();
			return readRequestD1();
			
		case READ_REQUEST_D1:
			return readRequestD1();
			
		case READ_WAIT_D1:
			return readWaitD1();
			
		case READ_GET_D1:
			return readGetD1();
			
		case READ_REQUEST_D2:
			return readRequestD2();
			
		case READ_WAIT_D2:
			return readWaitD2();
			
		case READ_GET_D2:
			return readGetD2();
			
		case READ_COMPLETE:
			_state = IDLE;
			return true;
			
		default:
			_state = IDLE;
			return false;
	}
}

float MS5837::pressure() {
	return P / 10.0f; // Convert to mbar
}

float MS5837::temperature() {
	return TEMP / 100.0f; // Convert to °C
}

float MS5837::depth() {
	if (surfaceCalibrated) {
		// Use calibrated surface pressure
		return (pressure() * 100.0f - surfacePressure) / (fluidDensity * 9.80665);
	} else {
		// Use standard atmospheric pressure
		return (pressure() * 100.0f - 101300) / (fluidDensity * 9.80665);
	}
}

// Private methods implementation
bool MS5837::initReset() {
	if (_state != INIT_RESET) return false;
	
	if (millis() - _operationStartTime < RESET_TIMEOUT) {
		return false;
	}
	
	_i2cPort->beginTransmission(MS5837_ADDR);
	_i2cPort->write(MS5837_RESET);
	_i2cPort->endTransmission();

	_state = INIT_READ_PROM;
	_operationStartTime = millis();
	return initReadPROM();
}

bool MS5837::initReadPROM() {
	if (_state != INIT_READ_PROM) return false;
	
	if (_promIndex < 7) {
		if (millis() - _operationStartTime < PROM_READ_TIMEOUT) {
			return false;
		}
		
		_i2cPort->beginTransmission(MS5837_ADDR);
		_i2cPort->write(MS5837_PROM_READ + _promIndex * 2);
		_i2cPort->endTransmission();

		_i2cPort->requestFrom(MS5837_ADDR, 2);
		C[_promIndex] = (_i2cPort->read() << 8) | _i2cPort->read();
		
		_promIndex++;
		_operationStartTime = millis();
		return false;
	}
	
	// Validate CRC
	uint8_t crcRead = C[0] >> 12;
	uint8_t crcCalculated = crc4(C);

	if (crcCalculated != crcRead) {
		_state = IDLE;
		return false;
	}

	_state = IDLE;
	_initComplete = true;
	return true;
}

bool MS5837::readRequestD1() {
	if (_state != READ_REQUEST_D1) return false;
	
	_i2cPort->beginTransmission(MS5837_ADDR);
	_i2cPort->write(MS5837_CONVERT_D1_8192);
	_i2cPort->endTransmission();

	_state = READ_WAIT_D1;
	_operationStartTime = millis();
	return false;
}

bool MS5837::readWaitD1() {
	if (_state != READ_WAIT_D1) return false;
	
	if (millis() - _operationStartTime < CONVERSION_TIMEOUT) {
		return false;
	}
	
	_state = READ_GET_D1;
	return readGetD1();
}

bool MS5837::readGetD1() {
	if (_state != READ_GET_D1) return false;
	
	_i2cPort->beginTransmission(MS5837_ADDR);
	_i2cPort->write(MS5837_ADC_READ);
	_i2cPort->endTransmission();

	_i2cPort->requestFrom(MS5837_ADDR, 3);
	D1_pres = (_i2cPort->read() << 16) | 
	          (_i2cPort->read() << 8) | 
	          _i2cPort->read();

	_state = READ_REQUEST_D2;
	_operationStartTime = millis();
	return readRequestD2();
}

bool MS5837::readRequestD2() {
	if (_state != READ_REQUEST_D2) return false;
	
	_i2cPort->beginTransmission(MS5837_ADDR);
	_i2cPort->write(MS5837_CONVERT_D2_8192);
	_i2cPort->endTransmission();

	_state = READ_WAIT_D2;
	_operationStartTime = millis();
	return false;
}

bool MS5837::readWaitD2() {
	if (_state != READ_WAIT_D2) return false;
	
	if (millis() - _operationStartTime < CONVERSION_TIMEOUT) {
		return false;
	}
	
	_state = READ_GET_D2;
	return readGetD2();
}

bool MS5837::readGetD2() {
	if (_state != READ_GET_D2) return false;
	
	_i2cPort->beginTransmission(MS5837_ADDR);
	_i2cPort->write(MS5837_ADC_READ);
	_i2cPort->endTransmission();

	_i2cPort->requestFrom(MS5837_ADDR, 3);
	D2_temp = (_i2cPort->read() << 16) | 
	          (_i2cPort->read() << 8) | 
	          _i2cPort->read();

	calculate();
	_state = READ_COMPLETE;
	return true;
}

void MS5837::calculate() {
	int32_t dT = D2_temp - uint32_t(C[5]) * 256l;
	
	// Determine sensor model based on sensitivity
	bool is30BA = (C[1] >= MS5837_30BA_MIN_SENSITIVITY && C[1] <= MS5837_02BA_30BA_SEPARATION);
	
	int64_t SENS, OFF;
	if (is30BA) {
		SENS = int64_t(C[1]) * 32768l + (int64_t(C[3]) * dT) / 256l;
		OFF = int64_t(C[2]) * 65536l + (int64_t(C[4]) * dT) / 128l;
		P = (D1_pres * SENS / (2097152l) - OFF) / (8192l);
	} else {
		SENS = int64_t(C[1]) * 65536l + (int64_t(C[3]) * dT) / 128l;
		OFF = int64_t(C[2]) * 131072l + (int64_t(C[4]) * dT) / 64l;
		P = (D1_pres * SENS / (2097152l) - OFF) / (32768l);
	}

	TEMP = 2000l + int64_t(dT) * C[6] / 8388608LL;

	// Second order temperature compensation
	int32_t SENSi = 0, OFFi = 0, Ti = 0;
	
	if (is30BA) {
		if ((TEMP / 100) < 20) {
			Ti = (3 * int64_t(dT) * int64_t(dT)) / (8589934592LL);
			OFFi = (3 * (TEMP - 2000) * (TEMP - 2000)) / 2;
			SENSi = (5 * (TEMP - 2000) * (TEMP - 2000)) / 8;
			if ((TEMP / 100) < -15) {
				OFFi = OFFi + 7 * (TEMP + 1500l) * (TEMP + 1500l);
				SENSi = SENSi + 4 * (TEMP + 1500l) * (TEMP + 1500l);
			}
		} else if ((TEMP / 100) >= 20) {
			Ti = 2 * (dT * dT) / (137438953472LL);
			OFFi = (1 * (TEMP - 2000) * (TEMP - 2000)) / 16;
			SENSi = 0;
		}
	} else {
		if ((TEMP / 100) < 20) {
			Ti = (11 * int64_t(dT) * int64_t(dT)) / (34359738368LL);
			OFFi = (31 * (TEMP - 2000) * (TEMP - 2000)) / 8;
			SENSi = (63 * (TEMP - 2000) * (TEMP - 2000)) / 32;
		}
	}

	int64_t OFF2 = OFF - OFFi;
	int64_t SENS2 = SENS - SENSi;
	TEMP = TEMP - Ti;

	if (is30BA) {
		P = ((D1_pres * SENS2) / 2097152l - OFF2) / 8192l;
	} else {
		P = ((D1_pres * SENS2) / 2097152l - OFF2) / 32768l;
	}
}

uint8_t MS5837::crc4(uint16_t n_prom[]) {
	uint16_t n_rem = 0;
	n_prom[0] = ((n_prom[0]) & 0x0FFF);
	n_prom[7] = 0;

	for (uint8_t i = 0; i < 16; i++) {
		if (i % 2 == 1) {
			n_rem ^= (uint16_t)((n_prom[i >> 1]) & 0x00FF);
		} else {
			n_rem ^= (uint16_t)(n_prom[i >> 1] >> 8);
		}
		for (uint8_t n_bit = 8; n_bit > 0; n_bit--) {
			if (n_rem & 0x8000) {
				n_rem = (n_rem << 1) ^ 0x3000;
			} else {
				n_rem = (n_rem << 1);
			}
		}
	}

	n_rem = ((n_rem >> 12) & 0x000F);
	return n_rem ^ 0x00;
}
