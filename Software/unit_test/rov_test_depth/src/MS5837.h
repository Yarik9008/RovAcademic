/* Оптимизированная библиотека MS5837 для STM32F103
------------------------------------------------------------

Title: Optimized MS5837 Pressure/Temperature Sensor Library

Description: Streamlined library for MS5837 sensor with only essential
functionality for depth measurement applications.

Authors: Optimized version based on Blue Robotics library

-------------------------------
The MIT License (MIT)

Copyright (c) 2015 Blue Robotics Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-------------------------------*/

#ifndef MS5837_H_OPTIMIZED
#define MS5837_H_OPTIMIZED

#include "Arduino.h"
#include <Wire.h>

class MS5837 {
public:
	// Constructor
	MS5837();

	// Core initialization
	bool init(TwoWire &wirePort = Wire);
	
	// Configuration
	void setFluidDensity(float density); // kg/m^3 (997 for freshwater, 1029 for seawater)
	
	// Calibration
	void calibrateSurface(); // Calibrate zero depth at surface
	
	// Non-blocking operations
	bool read(); // Returns true when reading complete
	bool isReady(); // Check if sensor ready for new reading
	void continueInit(); // Continue initialization process
	
	// Data access
	float pressure(); // Pressure in mbar
	float temperature(); // Temperature in °C
	float depth(); // Depth in meters (uses calibration if available)

private:
	// I2C communication
	TwoWire * _i2cPort;
	
	// Sensor data
	uint16_t C[8]; // Calibration coefficients
	uint32_t D1_pres, D2_temp; // Raw pressure and temperature data
	int32_t TEMP, P; // Calculated temperature and pressure
	
	// Configuration
	float fluidDensity;
	float surfacePressure; // Calibrated surface pressure
	bool surfaceCalibrated;
	
	// Non-blocking state machine
	enum State {
		IDLE,
		INIT_RESET,
		INIT_READ_PROM,
		READ_REQUEST_D1,
		READ_WAIT_D1,
		READ_GET_D1,
		READ_REQUEST_D2,
		READ_WAIT_D2,
		READ_GET_D2,
		READ_COMPLETE
	};
	
	State _state;
	unsigned long _operationStartTime;
	uint8_t _promIndex;
	bool _initComplete;
	
	// Internal methods
	void calculate(); // Perform sensor calculations
	uint8_t crc4(uint16_t n_prom[]); // CRC validation
	
	// State machine methods
	bool initReset();
	bool initReadPROM();
	bool readRequestD1();
	bool readWaitD1();
	bool readGetD1();
	bool readRequestD2();
	bool readWaitD2();
	bool readGetD2();
};

#endif
