# rov_test_serial_rx — приём строк по UART1 (StringUtils)

Тест приёма по **USART1**: строка собирается в буфер до `\n`, затем печатается в USB Serial через **[StringUtils](https://github.com/GyverLibs/StringUtils)** (`Text::printTo`).

## Платформа

| Параметр | Значение |
| -------- | -------- |
| Board | `bluepill_f103c8` |
| Зависимость | `gyverlibs/StringUtils` |
| Загрузка | DFU |

## Настройка (`src/config.h`)

Те же пины и скорость, что у передатчика: `PA9`/`PA10`, **57600** бод (`SERIAL_BAUD`).

## Сборка

Каталог сборки вынесен в `%TEMP%/pio_build/rov_test_serial_rx` (см. `platformio.ini`), чтобы линкер не падал на путях с кириллицей.

```bash
cd rov_test_serial_rx
pio run
pio run -t upload
pio device monitor -b 115200
```

Подайте на `Serial1` строки с переводом строки (например с [rov_test_serial_tx](../rov_test_serial_tx/README.md)).
