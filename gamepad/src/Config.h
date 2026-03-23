// режим отладки
const bool DEBUG = true;

// тест робота на выносливость
const bool TEST_ROBOT = false;

// подключение светодиода
const uint16_t LED_PIN = PC13;

// подключение UART
const uint16_t UART_RX = PA10;
const uint16_t UART_TX = PA9;
const uint16_t BITRATE = 57600;

// подключение джойстиков
const uint16_t JOYSTICK1_X = PA3;
const uint16_t JOYSTICK1_Y = PA4;
const uint16_t JOYSTICK2_X = PA5;
const uint16_t JOYSTICK2_Y = PA6;

// инверсия осей джойстиков
const bool INVERT_JOYSTICK1_X = false;
const bool INVERT_JOYSTICK1_Y = true;
const bool INVERT_JOYSTICK2_X = true;
const bool INVERT_JOYSTICK2_Y = false;

// подключение кнопок
const uint16_t BUTTON1 = PA15;
const uint16_t BUTTON2 = PB3;
const uint16_t BUTTON3 = PB4;
const uint16_t BUTTON4 = PB5;
const uint16_t BUTTON5 = PB6;
const uint16_t BUTTON6 = PB7;
const uint16_t BUTTON7 = PB8;
const uint16_t BUTTON8 = PB9;

// настройки ADC
const uint16_t ADC_MAX_VALUE = 4095;

// мертвая зона джойстика
const uint16_t DEAD_ZONE = 250;

// диапазон джойстика для нормализации
const uint16_t MIN_JOYSTIK_RANGE = 1000;
const uint16_t CENTER_JOYSTIK_RANGE = 1500;
const uint16_t MAX_JOYSTICK_RANGE = 2000;

// настройки фильтра Калмана
const float KALMAN_Q = 25.0;
const float KALMAN_R = 20.0;
const float KALMAN_P_INIT = 1000.0;

// интервалы таймеров (мс)
const uint16_t DATA_INTERVAL = 20;
const uint16_t LED_INTERVAL = 200;
const uint16_t CAMERA_UPDATE_INTERVAL = 100;

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
