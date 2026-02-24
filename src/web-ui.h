#pragma once
/**
 * @file web-ui.h
 * @brief Веб-интерфейс мониторинга процесса дистилляции Moonshine
 * 
 * Платформа: ESP8266 (NodeMCU v3)
 * 
 * Особенности:
 * - Комбинированный режим работы: точка доступа + подключение к Wi-Fi сети
 * - Локальное имя хоста: "moonshine"
 * - Реальное время: данные получаются из settings::moonshine::state_t
 * 
 * @note Для обновления макета веб-интерфейса:
 *       1. Взять актуальный файл из docs/web-ui/qwen-distillation.xxx.html
 *       2. Извлечь HTML/CSS/JS код
 *       3. Заменить данные симуляции на функцию get_state()
 *       4. Обновить версию в заголовке на MS_VERSION_STRING
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "settings.h"

namespace webui
{
    /**
     * @brief Конфигурация веб-сервера
     */
    struct config_t
    {
        const char* hostname = "moonshine";           // Имя хоста для mDNS
        const char* ap_ssid = "Moonshine_AP";         // SSID точки доступа
        const char* ap_password = "moonshine123";     // Пароль точки доступа
        const char* wifi_ssid = "";                   // SSID внешней Wi-Fi сети (пусто = только AP)
        const char* wifi_password = "";               // Пароль внешней Wi-Fi сети
        uint16_t port = 80;                           // Порт веб-сервера
        uint32_t update_interval = 500;               // Интервал обновления данных (мс)
    };

    /**
     * @brief Класс веб-сервера Moonshine
     * 
     * Предоставляет веб-интерфейс мониторинга процесса дистилляции.
     * Работает в режиме точки доступа или подключается к существующей Wi-Fi сети.
     */
    class MoonshineWebServer
    {
    public:
        /**
         * @brief Конструктор
         * @param cfg Конфигурация веб-сервера
         */
        explicit MoonshineWebServer(const config_t& cfg = config_t());

        /**
         * @brief Инициализация веб-сервера
         * 
         * - Настройка сети (AP или STA+AP)
         * - Регистрация обработчиков HTTP
         * - Запуск сервера
         * 
         * @return true при успешной инициализации
         */
        bool begin();

        /**
         * @brief Основной цикл обработки
         * 
         * Вызывать регулярно из loop() для обработки клиентских запросов
         */
        void handleClient();

        /**
         * @brief Проверка подключения к сети
         * @return true если подключено (AP или STA)
         */
        bool isConnected() const;

        /**
         * @brief Получить IP адрес
         * @return IP адрес в формате String
         */
        String getIPAddress() const;

        /**
         * @brief Получить режим работы
         * @return "AP" если только точка доступа, "STA" если клиент, "STA+AP" если оба режима
         */
        String getMode() const;

        /**
         * @brief Установить функцию получения состояния системы
         * @param getter Функция, возвращающая settings::moonshine::state_t
         */
        void setStateGetter(settings::moonshine::state_t (*getter)());

        /**
         * @brief Установить конфигурацию сервера
         * @param cfg Новая конфигурация
         * @note Должно быть вызвано до begin()
         */
        void setConfig(const config_t& cfg);

        /**
         * @brief Получить текущую конфигурацию
         * @return Конфигурация сервера
         */
        const config_t& getConfig() const { return m_config; }

    private:
        /**
         * @brief Обработчик корневой страницы
         * Отдаёт HTML страницу веб-интерфейса
         */
        void handleRoot();

        /**
         * @brief Обработчик API состояния
         * Отдаёт JSON с текущими показаниями датчиков
         */
        void handleApiState();

        /**
         * @brief Обработчик API статуса
         * Отдаёт JSON со статусом системы (uptime, режим работы и т.д.)
         */
        void handleApiStatus();

        /**
         * @brief Генерация JSON из состояния системы
         * @param state Состояние системы
         * @return JSON строка
         */
        String stateToJson(const settings::moonshine::state_t& state);

        config_t m_config;                          ///< Конфигурация
        ESP8266WebServer m_server;                  ///< Веб-сервер
        settings::moonshine::state_t (*m_stateGetter)() = nullptr;  ///< Функция получения состояния
        bool m_initialized = false;                 ///< Флаг инициализации
    };

    // Глобальный экземпляр сервера (опционально)
    extern MoonshineWebServer server;

} // namespace webui

/**
 * @brief Пример использования в main.cpp:
 *
 * @code
 * #include "web-ui.h"
 *
 * // Функция получения состояния (реализовать в main.cpp)
 * settings::moonshine::state_t get_state();
 *
 * webui::MoonshineWebServer webServer;
 * webui::config_t webConfig;
 *
 * void setup() {
 *     Serial.begin(115200);
 *
 *     // Настройка конфигурации
 *     webConfig.hostname = "moonshine";
 *     webConfig.ap_ssid = "Moonshine_AP";
 *     webConfig.ap_password = "moonshine123";
 *     webConfig.wifi_ssid = "MyWiFi";           // опционально
 *     webConfig.wifi_password = "MyPassword";   // опционально
 *
 *     // Установка конфигурации и инициализация
 *     webServer.setConfig(webConfig);
 *     if (webServer.begin()) {
 *         Serial.println("Web server started");
 *         Serial.print("IP: ");
 *         Serial.println(webServer.getIPAddress());
 *     }
 *
 *     // Установка функции получения состояния
 *     webServer.setStateGetter(get_state);
 * }
 *
 * void loop() {
 *     webServer.handleClient();
 *     // ... остальная логика
 * }
 * @endcode
 */
