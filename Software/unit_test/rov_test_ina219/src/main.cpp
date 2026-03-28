#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

// ========== КОНФИГУРАЦИЯ ПИНОВ ==========
// UART1 для вывода данных
#define UART_RX PA10
#define UART_TX PA9
#define SERIAL_BAUD_RATE 57600

// I2C для датчика тока INA219
#define I2C_SDA_PIN PB7
#define I2C_SCL_PIN PB6

// Встроенный светодиод
#define LED_BUILTIN_PIN PC13

// ========== ИНТЕРВАЛЫ ==========
#define DATA_PRINT_INTERVAL_MS 500  // Интервал вывода данных в UART (мс)

// ========== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ==========
Adafruit_INA219 ina219;
bool sensor_ok = false;
unsigned long last_print = 0;

// ========== ИНИЦИАЛИЗАЦИЯ ==========
void setup() {
    // Настройка UART1
    Serial.setRx(UART_RX);
    Serial.setTx(UART_TX);
    Serial.begin(SERIAL_BAUD_RATE);
    
    // Задержка для инициализации Serial
    delay(1000);
    
    // Настройка светодиода
    pinMode(LED_BUILTIN_PIN, OUTPUT);
    digitalWrite(LED_BUILTIN_PIN, LOW);
    
    // Инициализация I2C
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    
    // Инициализация INA219
    Serial.println("\n=== INA219 Test Firmware ===");
    Serial.println("Initializing INA219 sensor...");
    
    if (ina219.begin()) {
        sensor_ok = true;
        // Калибровка для диапазона ±32V, ±1A
        ina219.setCalibration_32V_1A();
        Serial.println("INA219 initialized successfully!");
        Serial.println("Calibration: 32V, 1A");
    } else {
        sensor_ok = false;
        Serial.println("ERROR: INA219 initialization failed!");
        Serial.println("Check I2C connections:");
        Serial.println("  SDA -> PB7");
        Serial.println("  SCL -> PB6");
    }
    
    Serial.println("\nStarting data polling...\n");
}

// ========== ОСНОВНОЙ ЦИКЛ ==========
void loop() {
    unsigned long now = millis();
    
    // Мигание светодиода каждые 500мс
    static unsigned long last_led = 0;
    static bool led_state = false;
    if (now - last_led >= DATA_PRINT_INTERVAL_MS) {
        led_state = !led_state;
        digitalWrite(LED_BUILTIN_PIN, led_state);
        last_led = now;
    }
    
    // Вывод данных с заданным интервалом
    if (now - last_print >= DATA_PRINT_INTERVAL_MS) {
        last_print = now;
        
        if (sensor_ok) {
            // Чтение данных с датчика
            float shunt_voltage_mV = ina219.getShuntVoltage_mV();
            float bus_voltage_V = ina219.getBusVoltage_V();
            float current_mA = ina219.getCurrent_mA();
            float power_mW = ina219.getPower_mW();
            float load_voltage_V = bus_voltage_V + (shunt_voltage_mV / 1000.0);
            
            // Вывод данных в UART1
            Serial.print("Time: ");
            Serial.print(now);
            Serial.print(" ms | ");
            
            Serial.print("Bus Voltage: ");
            Serial.print(bus_voltage_V, 3);
            Serial.print(" V | ");
            
            Serial.print("Shunt Voltage: ");
            Serial.print(shunt_voltage_mV, 3);
            Serial.print(" mV | ");
            
            Serial.print("Load Voltage: ");
            Serial.print(load_voltage_V, 3);
            Serial.print(" V | ");
            
            Serial.print("Current: ");
            Serial.print(current_mA, 2);
            Serial.print(" mA | ");
            
            Serial.print("Power: ");
            Serial.print(power_mW, 2);
            Serial.println(" mW");
            
        } else {
            Serial.println("Sensor error: INA219 not initialized!");
        }
    }
    
    // Небольшая задержка для стабильности
    delay(10);
}
