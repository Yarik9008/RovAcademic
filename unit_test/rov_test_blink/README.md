# rov_test_blink — мигание LED и USB Serial (Blue Pill)

Проверка платы **STM32F103C8** «Blue Pill»: встроенный светодиод **PC13** и вывод строк в **USB CDC Serial** (115200 бод).

## Платформа

| Параметр | Значение |
| -------- | -------- |
| Board | `bluepill_f103c8` |
| Загрузка | DFU (stm32duino bootloader) |
| Отладка | `Serial` — USB CDC при включённых флагах в `platformio.ini` |

На Blue Pill LED на `PC13` обычно активен **низким** уровнем: в коде «включено» = `LOW`.

## Сборка

```bash
cd rov_test_blink
pio run
pio run -t upload
pio device monitor -b 115200
```

## Ожидаемый результат

В мониторе порта чередуются строки `LED ON` / `LED OFF`; светодиод мигает с периодом ~1 с.
