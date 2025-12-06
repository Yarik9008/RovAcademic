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

// инвертирование направления моторов (true - инвертировать, false - не инвертировать)
const bool MOTOR_INVERT[4] = {true, false, false, false};

// подключение сервопривода камеры 
const uint16_t PIN_SERVO_CAM = PA7;

// подключение сервопривода манипулятора 
const uint16_t PIN_SERVO_ARM = PB0;

const float ACCEL_MOTORS = 4.0;
const uint16_t SPEED_MOTORS = 2500;

const uint16_t SPEED_SERVO = 70;
const float ACCELERATE_SERVO = 0.7;


