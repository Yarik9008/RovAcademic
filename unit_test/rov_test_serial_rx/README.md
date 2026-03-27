# rov_test_serial_rx — приём строк по UART1 (AsyncStream)

Тест приёма по **USART1**: библиотека **AsyncStream** накапливает данные до символа `\n` и печатает буфер в USB Serial.

## Платформа

| Параметр | Значение |
| -------- | -------- |
| Board | `bluepill_f103c8` |
| Зависимость | `gyverlibs/AsyncStream` |
| Загрузка | DFU |

## Настройка (`src/Config.h`)

Те же пины и скорость, что у передатчика: `PA9`/`PA10`, **57600** бод (`SERIAL_BAUD`).

## Сборка

```bash
cd rov_test_serial_rx
pio run
pio run -t upload
pio device monitor -b 115200
```

Подайте на `Serial1` строки с переводом строки (например с [rov_test_serial_tx](../rov_test_serial_tx/README.md)).
