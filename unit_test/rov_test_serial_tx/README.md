# rov_test_serial_tx — периодическая передача по UART1

Тест линии **USART1** (`Serial1`): каждые `TIMEOUT` мс (100 мс, см. `src/Config.h`) отправляется строка с монотонно растущим числом. Дублирование в **USB Serial** (`Serial`) для контроля.

## Платформа

| Параметр | Значение |
| -------- | -------- |
| Board | `bluepill_f103c8` |
| Загрузка | DFU |
| USB CDC | включён для отладочного порта |

## Настройка (`src/Config.h`)

| Параметр | Значение по умолчанию |
| -------- | --------------------- |
| `SERIAL1_TX` | `PA9` |
| `SERIAL1_RX` | `PA10` |
| `SERIAL_BAUD` | 57600 |
| `TIMEOUT` | 100 (мс) |

## Сборка

```bash
cd rov_test_serial_tx
pio run
pio run -t upload
pio device monitor -b 115200
```

Паруйте с [rov_test_serial_rx](../rov_test_serial_rx/README.md) на той же скорости и кроссом TX↔RX.
