#include "ROVMotor.h"

ROVMotor* ROVMotor::motors[MAX_MOTORS] = {nullptr};
int ROVMotor::motorCount = 0;

// Конструктор
ROVMotor::ROVMotor(int pin) 
    : escPin(pin), 
      currentSpeed(MOTOR_STOP), 
      targetSpeed(MOTOR_STOP), 
      speedChange(5), 
      lastUpdateTime(0), 
      updateInterval(10) {
}


// Инициализация мотора (аналог attach из Servo)
void ROVMotor::begin() {
    begin(MOTOR_MIN, MOTOR_MAX);
}

void ROVMotor::begin(int min, int max) {
    // Инициализация ESC через библиотеку Servo
    esc.attach(escPin, min, max);
    esc.writeMicroseconds(MOTOR_STOP);
}

// Основной метод обновления с встроенным таймером (в стиле ServoSmooth)
bool ROVMotor::tick() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastUpdateTime >= updateInterval) {
        if (currentSpeed != targetSpeed) {
            int difference = targetSpeed - currentSpeed;
            
            if (abs(difference) <= speedChange) {
                currentSpeed = targetSpeed;
            } else {
                if (difference > 0) {
                    currentSpeed += speedChange;
                } else {
                    currentSpeed -= speedChange;
                }
            }
            
            // Отправляем команду ESC через библиотеку Servo
            esc.writeMicroseconds(currentSpeed);
        }
        
        lastUpdateTime = currentTime;
        return (currentSpeed == targetSpeed);
    }
    
    return false;
}

// Установка целевой скорости в микросекундах
void ROVMotor::setTarget(int target) {
    targetSpeed = constrain(target, MOTOR_MIN, MOTOR_MAX);
}

// Установка скорости в процентах (-100 до +100)
void ROVMotor::setTargetPercent(int percent) {
    percent = constrain(percent, -100, 100);
    int target;
    
    if (percent == 0) {
        target = MOTOR_STOP;
    } else if (percent > 0) {
        // Положительный процент: от остановки до максимума
        target = map(percent, 0, 100, MOTOR_STOP, MOTOR_MAX);
    } else {
        // Отрицательный процент: от остановки до минимума
        target = map(percent, -100, 0, MOTOR_MIN, MOTOR_STOP);
    }
    
    setTarget(target);
}

// Установка максимальной скорости изменения
void ROVMotor::setSpeed(int speed) {
    speedChange = constrain(speed, 1, 50);
}

// Остановка мотора
void ROVMotor::stop() {
    setTarget(MOTOR_STOP);
}

// Получение текущей скорости в микросекундах
int ROVMotor::getCurrent() const {
    return currentSpeed;
}

// Получение целевой скорости в микросекундах
int ROVMotor::getTarget() const {
    return targetSpeed;
}

// Получение текущей скорости в процентах
int ROVMotor::getCurrentPercent() const {
    if (currentSpeed == MOTOR_STOP) {
        return 0;
    } else if (currentSpeed > MOTOR_STOP) {
        return map(currentSpeed, MOTOR_STOP, MOTOR_MAX, 0, 100);
    } else {
        return map(currentSpeed, MOTOR_MIN, MOTOR_STOP, -100, 0);
    }
}

// Получение целевой скорости в процентах
int ROVMotor::getTargetPercent() const {
    if (targetSpeed == MOTOR_STOP) {
        return 0;
    } else if (targetSpeed > MOTOR_STOP) {
        return map(targetSpeed, MOTOR_STOP, MOTOR_MAX, 0, 100);
    } else {
        return map(targetSpeed, MOTOR_MIN, MOTOR_STOP, -100, 0);
    }
}

// Проверка, достигнута ли целевая скорость
bool ROVMotor::isTargetReached() const {
    return currentSpeed == targetSpeed;
}

// Статические методы для управления массивом моторов

// Добавление мотора в статический массив
bool ROVMotor::addMotor(int pin) {
    if (motorCount >= MAX_MOTORS) {
        return false; // Достигнуто максимальное количество моторов
    }
    
    // Создаем новый мотор
    motors[motorCount] = new ROVMotor(pin);
    motorCount++;
    
    return true;
}

// Инициализация всех моторов
void ROVMotor::beginAll() {
    for (int i = 0; i < motorCount; i++) {
        if (motors[i] != nullptr) {
            motors[i]->begin();
        }
    }
}

// Установка скорости для конкретного мотора в микросекундах
bool ROVMotor::setMotorSpeed(int motorIndex, int speed) {
    if (motorIndex < 0 || motorIndex >= motorCount || motors[motorIndex] == nullptr) {
        return false; // Неверный индекс или мотор не существует
    }
    
    motors[motorIndex]->setTarget(speed);
    return true;
}

// Установка скорости для конкретного мотора в процентах
bool ROVMotor::setMotorPercent(int motorIndex, int percent) {
    if (motorIndex < 0 || motorIndex >= motorCount || motors[motorIndex] == nullptr) {
        return false; // Неверный индекс или мотор не существует
    }
    
    motors[motorIndex]->setTargetPercent(percent);
    return true;
}

// Обновление всех моторов с таймером
bool ROVMotor::tickAll() {
    bool allReached = true;
    
    for (int i = 0; i < motorCount; i++) {
        if (motors[i] != nullptr) {
            if (!motors[i]->tick()) {
                allReached = false;
            }
        }
    }
    
    return allReached;
}

// Получение текущей скорости мотора
int ROVMotor::getMotorCurrentSpeed(int motorIndex) {
    if (motorIndex < 0 || motorIndex >= motorCount || motors[motorIndex] == nullptr) {
        return -1; // Неверный индекс или мотор не существует
    }
    
    return motors[motorIndex]->getCurrent();
}

// Получение целевой скорости мотора
int ROVMotor::getMotorTargetSpeed(int motorIndex) {
    if (motorIndex < 0 || motorIndex >= motorCount || motors[motorIndex] == nullptr) {
        return -1; // Неверный индекс или мотор не существует
    }
    
    return motors[motorIndex]->getTarget();
}

// Проверка, достигнута ли целевая скорость для мотора
bool ROVMotor::isMotorTargetReached(int motorIndex) {
    if (motorIndex < 0 || motorIndex >= motorCount || motors[motorIndex] == nullptr) {
        return false; // Неверный индекс или мотор не существует
    }
    
    return motors[motorIndex]->isTargetReached();
}
