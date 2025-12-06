#ifndef CONFIG_H
#define CONFIG_H

// Serial порты
#define SERIAL1_RX PA10
#define SERIAL1_TX PA9   

// Джойстики
#define JOYSTICK1_X PA2
#define JOYSTICK1_Y PA3
#define JOYSTICK2_X PA5
#define JOYSTICK2_Y PA4

// Кнопки
#define BUTTON1 PB12
#define BUTTON2 PB13
#define BUTTON3 PB14
#define BUTTON4 PB15
#define BUTTON5 PB6
#define BUTTON6 PB7
#define BUTTON7 PB8
#define BUTTON8 PB9

// Светодиод
#define LED_PIN PC13

// ADC
#define ADC_MAX_VALUE 4095

// Serial скорости
#define SERIAL_BAUD 57600
#define DEBUG_BAUD 115200

// DEBAG
#define DEBUG_JOYSTICK true

// Фильтр Калмана
#define KALMAN_Q 25
#define KALMAN_R 20
#define KALMAN_P_INIT 1000

// Таймеры
#define DATA_INTERVAL 20
#define LED_INTERVAL 200
#define CAMERA_UPDATE_INTERVAL 100

// Мертвая зона джойстика
#define DEAD_ZONE 250

// Диапазон джойстика для нормализации
#define MAX_JOYSTICK_RANGE 2000
#define CENTER_JOYSTIK_RANGE 1500
#define MIN_JOYSTIK_RANGE 1000


// Структуры
struct JoystickCoefficients {
    float stick1_x = 0;
    float stick1_y = 0.7;
    float stick2_x = 0.5;
    float stick2_y = 0.7;
};

struct KalmanFilter {
    float x;
    float P;
    float K;
    float Q;
    float R;
};

struct JoystickData {
    float joy1X;
    float joy1Y;
    float joy2X;
    float joy2Y;
    int button1;
    int button2;
    int button3;
    int button4;
    int button5;
    int button6;
    int button7;
    int button8;
};

#endif // CONFIG_H