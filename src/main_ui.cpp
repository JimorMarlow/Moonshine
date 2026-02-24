/**
 * @file main_ui.cpp
 * @brief Пример инициализации и запуска веб-интерфейса Moonshine
 * 
 * Этот файл содержит пример кода для инициализации веб-сервера.
 * Перенесите нужный код в main.cpp по мере необходимости.
 * 
 * @note Не забудьте добавить вызов webServer.handleClient() в основной цикл loop()
 */

#include <Arduino.h>
#include "version.h"
#include "settings.h"
#include "web-ui.h"

// ============================================================
// Конфигурация веб-сервера
// ============================================================
// Откомментируйте и настройте для подключения к вашей Wi-Fi сети
// Если не настроено, сервер будет работать только в режиме точки доступа

webui::config_t webConfig;
// const char* WIFI_SSID = "YourWiFiSSID";        // Имя вашей Wi-Fi сети
// const char* WIFI_PASSWORD = "YourWiFiPassword";// Пароль вашей Wi-Fi сети

// ============================================================
// Глобальный экземпляр веб-сервера
// ============================================================
webui::MoonshineWebServer webServer;

// ============================================================
// Функция получения состояния системы
// ============================================================
// В реальной реализации эта функция должна возвращать актуальные
// данные от датчиков. Сейчас возвращает тестовые значения.

settings::moonshine::state_t get_state()
{
    settings::moonshine::state_t state;

    // Время работы от начала (в миллисекундах)
    state.uptime_ms = millis();

    // Тестовые значения температур (замените на реальные данные)
    // Для демонстрации используем синусоидальные колебания
    static uint32_t lastUpdate = 0;
    static float baseTemp = 25.0;

    if (millis() - lastUpdate > 1000) {
        lastUpdate = millis();
        // Плавное изменение базовой температуры для демонстрации
        baseTemp = 25.0 + 10.0 * sin(millis() / 10000.0);
    }

    // Температура пара (верх колонны)
    state.steam_temperature = baseTemp + 5.0;

    // Температура в нагревательном баке
    state.heater_temperature = baseTemp + 10.0;

    // Температура воды на выходе дефлегматора
    state.deflegmater_temperature = baseTemp - 5.0;

    // Температура воды на выходе конденсера
    state.condenser_temperature = baseTemp - 8.0;

    // Поток воды (ml/min)
    state.deflegmater_water_flow = 250.0 + 50.0 * sin(millis() / 5000.0);
    state.condenser_water_flow = 230.0 + 40.0 * sin(millis() / 5000.0);

    // Отрицательные значения потока = 0 (нет потока)
    if (state.deflegmater_water_flow < 0) state.deflegmater_water_flow = 0;
    if (state.condenser_water_flow < 0) state.condenser_water_flow = 0;

    // Для отладки выводим состояние в Serial раз в 5 секунд
    static uint32_t lastTrace = 0;
    if (millis() - lastTrace > 5000) {
        lastTrace = millis();
        state.trace();
    }

    return state;
}

// ============================================================
// Инициализация
// ============================================================
void setup_ui()
{
    Serial.println(F("\n=== Moonshine Web UI Setup ==="));
    Serial.print(F("Firmware version: "));
    Serial.println(MS_VERSION_STRING);

    // Настройка конфигурации веб-сервера
    webConfig.hostname = "moonshine";
    webConfig.ap_ssid = "Moonshine_AP";
    webConfig.ap_password = "moonshine123";
    webConfig.port = 80;
    webConfig.update_interval = 500; // 500ms

    // Раскомментировать для подключения к Wi-Fi сети
    // webConfig.wifi_ssid = WIFI_SSID;
    // webConfig.wifi_password = WIFI_PASSWORD;

    // Инициализация веб-сервера
    if (!webServer.begin()) {
        Serial.println(F("[ERROR] Web server initialization failed!"));
        return;
    }

    // Установка функции получения состояния
    webServer.setStateGetter(get_state);

    // Вывод информации о подключении
    Serial.println(F("\n=== Web Server Info ==="));
    Serial.print(F("Mode: "));
    Serial.println(webServer.getMode());
    Serial.print(F("IP Address: "));
    Serial.println(webServer.getIPAddress());
    Serial.print(F("Hostname: http://"));
    Serial.print(webServer.getMode() == "AP" ? webServer.getIPAddress() : "moonshine.local");
    Serial.println(F("/"));
    Serial.println(F("=========================\n"));
}

// ============================================================
// Основной цикл обработки веб-сервера
// ============================================================
void loop_ui()
{
    // Обработка клиентских запросов
    webServer.handleClient();

    // Здесь может быть другая логика вашего приложения
    // ...
}

// ============================================================
// Пример использования в main.cpp:
// ============================================================
/*
#include "main_ui.h" // или подключить функции напрямую

void setup() {
    Serial.begin(115200);
    delay(1000);

    // ... инициализация датчиков и другого оборудования ...

    // Инициализация веб-интерфейса
    setup_ui();
}

void loop() {
    // Обработка веб-запросов
    loop_ui();

    // ... остальная логика приложения ...
}
*/
