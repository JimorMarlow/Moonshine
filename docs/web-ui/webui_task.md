# Разработка web-ui интерфейся для контроля дистилляции

## Макет
Взять последний актуальный макет web-ui интрерфейса из папки docs\web-ui\qwen-distillation.xxx.html
и на его основе разработать реализацию
- Платформа ESP8266 (посмотри platformio.ini, конфигурация nodemcuv3).
- Комбинированный режим работы: отдельная точка доступа с локальным именем хоста moonshine или подключение к существующей wi-fi сети по имени точки доступа и паролем
- из макета убрать всю логику иммитации

## Файлы и реализация
- создать два файла src\web-ui.h, src\web-ui.cpp с реализацией сервера, и src\main_ui.cpp с примером кода для инициализации и запуска локального сервера.
- src\main.cpp не трогать, я туда сам перенесу нужный код из src\main_ui.cpp
- Предусмотреть в src\web-ui.h, src\web-ui.cpp комментарции, чтобы можно было обновить код при изменении макета из нового файла docs\web-ui\qwen-distillation.xxx.html

## Входные данные
- src\settings.h для генерации новой страницы брать данные из структуры settings::moonshine::state_t
- актуальные данные я сам буду наполнять, сейчас в src\main_ui.cpp просто сделай функцию, которая будет выдавать какое-то тестовое значение
settings::moonshine::state_t get_state();

## Версия веб интерфейса
В заголовке "Moonshine vX.X.X" должно показываться актуальное значение MS_VERSION_STRING из version.h

## Readme
После заверешения обнови текст в файле README.md в разделе "### Version"

## История разработки

### История изменений (завершённые этапы)

#### Этап 1: Базовая реализация веб-сервера (v1.2.0) ✅
**Дата:** 2026-02-24

**Созданные файлы:**
- `src/web-ui.h` — объявление класса MoonshineWebServer и конфигурации
- `src/web-ui.cpp` — реализация сервера с HTML шаблоном (на базе qwen-distillation.043.html)
- `src/main_ui.cpp` — пример инициализации с тестовой функцией get_state()

**Функционал:**
- Комбинированный режим WiFi (AP + STA)
- mDNS: http://moonshine.local
- HTML страница с SVG схемой дистилляционной установки
- AJAX обновление данных каждые 500ms
- API endpoints:
  - `GET /` — HTML страница
  - `GET /api/state` — JSON с данными датчиков
  - `GET /api/status` — JSON со статусом системы

**Технические детали:**
- HTML шаблон хранится в PROGMEM для экономии RAM
- Версия в заголовке синхронизирована с MS_VERSION_STRING
- Данные берутся из settings::moonshine::state_t
- Тестовая функция get_state() возвращает синусоидальные колебания температур

**Использование:**
```cpp
#include "web-ui.h"

webui::MoonshineWebServer webServer;

void setup() {
    webServer.setStateGetter(get_state);
    webServer.begin();
}

void loop() {
    webServer.handleClient();
}
```

**Обновления в документации:**
- README.md: добавлен раздел "1.2.0 Web UI для ESP8266"
- WEB_UI-HISTORY.md: правило о создании новых файлов с увеличенным номером

---

### Примечания для продолжения работы

**Для обновления макета веб-интерфейса:**
1. Взять актуальный HTML из `docs/web-ui/qwen-distillation.xxx.html`
2. Извлечь CSS из `<style>` и JS из `<script>`
3. В `src/web-ui.cpp`:
   - Заменить HTML_PAGE на новый шаблон
   - Удалить логику симуляции (simState, sensorData, updateSimulationLogic)
   - Оставить только функцию обновления UI из JSON (updateSensors)
   - Проверить замену `${VERSION}` на MS_VERSION_STRING

**Текущая версия макета:** qwen-distillation.043.html (v1.1.12)

**Известные ограничения:**
- HTML шаблон сжат (удалены комментарии, пробелы) для экономии места
- При добавлении новых элементов в UI обновить updateSensors() в JavaScript
- Для новых датчиков добавить поля в settings::moonshine::state_t и обработать в stateToJson()