# Тестовые прошивки (unit_test)

Отдельные минимальные PlatformIO-проекты для проверки железа на стенде. Каждый каталог — **самостоятельный** проект: `cd <каталог>` → `pio run`. Подробности по пинам, скорости порта и загрузке — в **README.md** соответствующего каталога.

| Каталог | Назначение |
|---------|------------|
| [pwm_generator/](pwm_generator/README.md) | Серво-тест, **STM32F446**, пин **PB1** |
| [pwm_test_nano/](pwm_test_nano/README.md) | Серво-тест, **Arduino Nano**, пин **D3** |
| [rov_test_blink/](rov_test_blink/README.md) | Мигание **PC13** + USB Serial |
| [rov_test_depth/](rov_test_depth/README.md) | Датчик глубины **MS5837** (I2C) |
| [rov_test_i2c/](rov_test_i2c/README.md) | Сканер I2C, лог на USART1 |
| [rov_test_imu/](rov_test_imu/README.md) | IMU **BNO085** (I2C) |
| [rov_test_ina219/](rov_test_ina219/README.md) | **INA219**, лог на UART1 |
| [rov_test_motor/](rov_test_motor/README.md) | 4× PWM моторы, **Arduino Nano**, класс ROVMotor |
| [rov_test_serial_rx/](rov_test_serial_rx/README.md) | Приём по UART (**StringUtils** `Text`) |
| [rov_test_serial_tx/](rov_test_serial_tx/README.md) | Передача счётчика по UART1 |
