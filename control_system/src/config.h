// режим отладки
const bool DEBUG = true;

// подключение светодиода
const uint16_t LED_PIN = PC13;

// подключение UART
const uint16_t UART_RX = PA10;
const uint16_t UART_TX = PA9;
const uint16_t BITRATE = 57600;

// подключение моторов 
const uint16_t PIN_MOTOR_1 = PA6;
const uint16_t PIN_MOTOR_2 = PA5;
const uint16_t PIN_MOTOR_3 = PA4;
const uint16_t PIN_MOTOR_4 = PA3;

// инвертирование направления моторов 
const bool MOTOR_INVERT[4] = {true, false, false, false};

// подключение сервопривода камеры 
const uint16_t PIN_SERVO_CAM = PA7;

// подключение сервопривода манипулятора 
const uint16_t PIN_SERVO_ARM = PB0;

const float ACCEL_MOTORS = 4.0;
const uint16_t SPEED_MOTORS = 2500;

const uint16_t SPEED_SERVO = 70;
const float ACCELERATE_SERVO = 0.7;

// таймаут проверки получения новых данных (мс)
const uint32_t DATA_TIMEOUT = 1000;

// Константы для управления сервоприводами
const uint16_t SERVO_MIN = 1000;
const uint16_t SERVO_MAX = 2000;
const uint16_t SERVO_CENTER = 1500;

// Константы для манипулятора
const uint16_t GRIP_OPEN = 2000;
const uint16_t GRIP_CLOSE = 1000;
const uint16_t ARM_SPEED = 500;
const float ARM_ACCEL = 1.0;
const uint16_t ARM_INIT_ANGLE = 90;

// Константы для камеры
const int16_t CAM_STEP = 20;  // шаг изменения позиции камеры

// Константы для таймеров
const uint16_t SERVO_UPDATE_INTERVAL = 10;  // интервал обновления сервоприводов (мс)
const uint16_t LED_BLINK_INTERVAL = 100;    // интервал мигания LED (мс)

// Количество ожидаемых данных от пульта управления
const uint8_t EXPECTED_DATA_COUNT = 9;

// Размер буфера для входных данных
const uint8_t INPUT_DATA_BUFFER_SIZE = 10;

