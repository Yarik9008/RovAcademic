#include <Arduino.h>
#include "Config.h"

// Калибровочные значения (центр джойстиков)
int joy1X_zero = ADC_MAX_VALUE / 2;
int joy1Y_zero = ADC_MAX_VALUE / 2;
int joy2X_zero = ADC_MAX_VALUE / 2;
int joy2Y_zero = ADC_MAX_VALUE / 2;

// Фильтры Калмана для каждого джойстика
KalmanFilter kalman_joy1X = {ADC_MAX_VALUE / 2, KALMAN_P_INIT, 0, KALMAN_Q, KALMAN_R};
KalmanFilter kalman_joy1Y = {ADC_MAX_VALUE / 2, KALMAN_P_INIT, 0, KALMAN_Q, KALMAN_R};
KalmanFilter kalman_joy2X = {ADC_MAX_VALUE / 2, KALMAN_P_INIT, 0, KALMAN_Q, KALMAN_R};
KalmanFilter kalman_joy2Y = {ADC_MAX_VALUE / 2, KALMAN_P_INIT, 0, KALMAN_Q, KALMAN_R};

uint32_t ledTimer;
int ledState = LOW;


// Применение фильтра Калмана
float applyKalmanFilter(KalmanFilter &kf, float measurement) {
    // Предсказание
    kf.P = kf.P + kf.Q;
    // Обновление коэффициента Калмана
    kf.K = kf.P / (kf.P + kf.R);
    // Коррекция оценки
    kf.x = kf.x + kf.K * (measurement - kf.x);
    // Обновление ковариации
    kf.P = (1 - kf.K) * kf.P;
    return kf.x;
}

// Чтение и нормализация джойстика с фильтрацией Калмана
int readJoystick(int pin, int zero, KalmanFilter &kf, float coefficient) {
    // Опрос пина
    int raw = analogRead(pin);
    
    // Применение фильтра Калмана
    float filtered = applyKalmanFilter(kf, raw);
    
    // Учет центрального значения
    int offset = (int)filtered - zero;
    
    // Мертвая зона
    if (abs(offset) < DEAD_ZONE) offset = 0;
    
    // Ограничение диапазона
    offset = constrain(offset, -MAX_JOYSTICK_RANGE, MAX_JOYSTICK_RANGE);
    
    // Применение коэффициента вклада
    offset = (int)(offset * coefficient);
    
    // Преобразование в диапазон MIN_JOYSTIK_RANGE - MAX_JOYSTICK_RANGE
    // где CENTER_JOYSTIK_RANGE = центр
    int range = MAX_JOYSTICK_RANGE - MIN_JOYSTIK_RANGE;
    return CENTER_JOYSTIK_RANGE + (offset * range) / (MAX_JOYSTICK_RANGE * 2);
}

// Чтение всех данных
void readData(int* joy, int* buttons) {
    // Статический экземпляр коэффициентов
    static const JoystickCoefficients coeffs;
    
    // Джойстики (значения в диапазоне MIN_JOYSTIK_RANGE - MAX_JOYSTICK_RANGE)
    joy[0] = readJoystick(JOYSTICK1_X, joy1X_zero, kalman_joy1X, coeffs.stick1_x);  // joy1X
    joy[1] = readJoystick(JOYSTICK1_Y, joy1Y_zero, kalman_joy1Y, coeffs.stick1_y);  // joy1Y
    joy[2] = readJoystick(JOYSTICK2_X, joy2X_zero, kalman_joy2X, coeffs.stick2_x);  // joy2X
    joy[3] = readJoystick(JOYSTICK2_Y, joy2Y_zero, kalman_joy2Y, coeffs.stick2_y);  // joy2Y
    
    // Тест робота на выносливость 
    if (TEST_ROBOT) {
        joy[1] = 1250;
        joy[3] = 1750;
    }

    // Кнопки
    buttons[0] = (!digitalRead(BUTTON1) && digitalRead(BUTTON2)) ? -1 : 
                 (digitalRead(BUTTON1) && !digitalRead(BUTTON2)) ? 1 : 0;  // servo_cam
    buttons[1] = (!digitalRead(BUTTON3) && digitalRead(BUTTON4)) ? -1 : 
                 (digitalRead(BUTTON3) && !digitalRead(BUTTON4)) ? 1 : 0;  // gripper
    buttons[2] = (!digitalRead(BUTTON5) && digitalRead(BUTTON6)) ? 1 : 
                 (digitalRead(BUTTON5) && !digitalRead(BUTTON6)) ? -1 : 0;  // led
    buttons[3] = !digitalRead(BUTTON7) ? 1 : 0;  // button7
    buttons[4] = !digitalRead(BUTTON8) ? 1 : 0;  // button8
}

void setup() {
    // Настройка пинов
    pinMode(LED_PIN, OUTPUT);
    
    const int buttonPins[] = {BUTTON1, BUTTON2, BUTTON3, BUTTON4, BUTTON5, BUTTON6, BUTTON7, BUTTON8};
    for (int i = 0; i < 8; i++) {
        pinMode(buttonPins[i], INPUT_PULLUP);
    }
    
    // Настройка ADC
    analogReadResolution(12);
    
    // Калибровка джойстиков (чтение центральных значений)
    delay(100);  // Небольшая задержка для стабилизации
    joy1X_zero = analogRead(JOYSTICK1_X);
    joy1Y_zero = analogRead(JOYSTICK1_Y);
    joy2X_zero = analogRead(JOYSTICK2_X);
    joy2Y_zero = analogRead(JOYSTICK2_Y);
    
    // Инициализация фильтров Калмана центральными значениями
    kalman_joy1X.x = joy1X_zero;
    kalman_joy1Y.x = joy1Y_zero;
    kalman_joy2X.x = joy2X_zero;
    kalman_joy2Y.x = joy2Y_zero;
    
    // Инициализация Serial
    Serial.begin(DEBUG_BAUD);
    Serial1.setRx(SERIAL1_RX);
    Serial1.setTx(SERIAL1_TX);
    Serial1.begin(SERIAL_BAUD);
    
    // костыль, чтобы робот раздуплился 
    if (TEST_ROBOT) {
        delay(10000);
    }

    Serial.println("ROV Gamepad Ready");
}

void loop() {
    static unsigned long lastTime = 0;
    unsigned long currentTime = millis();

    // мигалка для индикации работы (период 500 мс)
    if (millis() - ledTimer >= 500){
        ledTimer = millis();
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
        // if (DEBUG) Serial.println(ledState);
    }
    
    if (currentTime - lastTime >= DATA_INTERVAL) {
        int joy[6];
        int buttons[5];
        
        readData(joy, buttons);
        
        if (DEBUG_JOYSTICK) { 
            // Вывод данных через Serial (отладка)
            Serial.print("joy1X:"); Serial.print(joy[0]); Serial.print(" ");
            Serial.print("joy1Y:"); Serial.print(joy[1]); Serial.print(" ");
            Serial.print("joy2X:"); Serial.print(joy[2]); Serial.print(" ");
            Serial.print("joy2Y:"); Serial.print(joy[3]); Serial.print(" ");
            Serial.print("CAM:"); Serial.print(buttons[0]); Serial.print(" ");
            Serial.print("GRIP:"); Serial.print(buttons[1]); Serial.print(" ");
            Serial.print("LED:"); Serial.print(buttons[2]); Serial.print(" ");
            Serial.print("B7:"); Serial.print(buttons[3]); Serial.print(" ");
            Serial.print("B8:"); Serial.println(buttons[4]);
        }
        
        // Вывод данных через Serial1
        Serial1.print(joy[0]); Serial1.print(" ");
        Serial1.print(joy[1]); Serial1.print(" ");
        Serial1.print(joy[2]); Serial1.print(" ");
        Serial1.print(joy[3]); Serial1.print(" ");
        Serial1.print(buttons[0]); Serial1.print(" ");
        Serial1.print(buttons[1]); Serial1.print(" ");
        Serial1.print(buttons[2]); Serial1.print(" ");
        Serial1.print(buttons[3]); Serial1.print(" ");
        Serial1.println(buttons[4]);
        
        lastTime = currentTime;
    }
}
