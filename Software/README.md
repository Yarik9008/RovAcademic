# Software

Прошивки и вспомогательные **PlatformIO**-проекты ТНПА Академик.

## Состав


| Каталог                            | Назначение      | Документация                                         |
| ---------------------------------- | --------------- | ---------------------------------------------------- |
| [control_system/](control_system/) | Борт ROV        | [control_system/README.md](control_system/README.md) |
| [gamepad/](gamepad/)               | Пульт           | [gamepad/README.md](gamepad/README.md)               |
| [gripper/](gripper/)               | Манипулятор     | [gripper/README.md](gripper/README.md)               |
| [unit_test/](unit_test/)           | Тестовые скетчи | [unit_test/README.md](unit_test/README.md)           |


## Сборка

Из **корня репозитория** `rov_academic`:

```bash
cd Software/<проект>
pio run
pio run -t upload
```

Общее описание архитектуры и протокола — в [README.md](../README.md) в корне репозитория.