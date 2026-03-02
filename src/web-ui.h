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
#include "version.h"
#include "etl_wifi_setup.h"

namespace webui
{
    /**
     * @brief Конфигурация веб-сервера Moonshine
     *
     * Наследуется от etl::wifi::server_config_t с проектными значениями по умолчанию.
     */
    struct moonshine_server_config_t : public etl::wifi::server_config_t
    {
        moonshine_server_config_t()
        {
            hostname = "moonshine";              // Имя хоста для mDNS
            ap_ssid = "Moonshine_AP";            // SSID точки доступа
            ap_password = "moonshine123";        // Пароль точки доступа
            wifi_ssid = "";                      // SSID внешней Wi-Fi сети (пусто = только AP)
            wifi_password = "moonshine123";      // Пароль внешней Wi-Fi сети
            port = 80;                           // Порт веб-сервера
            update_interval = 500;               // Интервал обновления данных (мс)

            device_name = "Moonshine v" + String(MS_VERSION_STRING);   // Название устройства
            device_description = "Контроль температуры дистилляционной колонны на базе ESP8266";  // Описание
            
            // SVG иконка устройства (из docs/images/icon_moonshine.svg)
            // Упрощённая иконка для встраивания в прошивку
            device_icon_svg = F("<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 33.6 39.7\"><rect width=\"33.6\" height=\"39.7\" fill=\"#a2d6fd\" rx=\"4\"/><circle cx=\"16.8\" cy=\"12\" r=\"5\" fill=\"#1d436d\"/><path d=\"M16.8 18v14M12 25h9.6\" stroke=\"#1d436d\" stroke-width=\"2\" stroke-linecap=\"round\"/><circle cx=\"16.8\" cy=\"32\" r=\"2.5\" fill=\"#1d436d\"/></svg>");
        }
    };

    /**
     * @brief Класс веб-сервера Moonshine
     *
     * Предоставляет веб-интерфейс мониторинга процесса дистилляции.
     * Работает в режиме точки доступа или подключается к существующей Wi-Fi сети.
     */
    class moonshine_web_server
    {
    public:
        /**
         * @brief Конструктор
         * @param cfg Конфигурация веб-сервера
         */
        explicit moonshine_web_server(const etl::wifi::server_config_t& cfg = moonshine_server_config_t());

        /**
         * @brief Деструктор
         *
         * Виртуальный деструктор для корректного наследования
         */
        virtual ~moonshine_web_server();

        /**
         * @brief Инициализация веб-сервера
         *
         * - Настройка сети (AP или STA+AP)
         * - Регистрация обработчиков HTTP
         * - Запуск сервера
         *
         * @return true при успешной инициализации
         */
        virtual bool begin();

        /**
         * @brief Основной цикл обработки
         *
         * Вызывать регулярно из loop() для обработки клиентских запросов
         */
        virtual void handle_client();

        /**
         * @brief Остановка веб-сервера
         *
         * Виртуальный метод для корректного наследования
         *
         * - Остановка HTTP сервера
         * - Отключение mDNS
         * - Отключение WiFi
         * - Сброс флага инициализации
         */
        virtual void stop();

        /**
         * @brief Проверка подключения к сети
         * @return true если подключено (AP или STA)
         */
        virtual bool is_connected() const;

        /**
         * @brief Получить IP адрес
         * @return IP адрес в формате String
         */
        virtual String get_ip_address() const;

        /**
         * @brief Получить режим работы
         * @return "AP" если только точка доступа, "STA" если клиент, "STA+AP" если оба режима
         */
        virtual String get_mode() const;

        /**
         * @brief Установить функцию получения состояния системы
         * @param getter Функция, возвращающая settings::moonshine::state_t
         */
        virtual void set_state_getter(settings::moonshine::state_t (*getter)());

        /**
         * @brief Установить конфигурацию сервера
         * @param cfg Новая конфигурация
         * @note Должно быть вызвано до begin()
         */
        virtual void set_config(const etl::wifi::server_config_t& cfg);

        /**
         * @brief Получить текущую конфигурацию
         * @return Конфигурация сервера
         */
        virtual const etl::wifi::server_config_t& get_config() const { return m_config; }

    protected:
        /**
         * @brief Обработчик корневой страницы
         * Отдаёт HTML страницу веб-интерфейса
         */
        virtual void handle_root();

        /**
         * @brief Обработчик API состояния
         * Отдаёт JSON с текущими показаниями датчиков
         */
        virtual void handle_api_state();

        /**
         * @brief Обработчик API статуса
         * Отдаёт JSON со статусом системы (uptime, режим работы и т.д.)
         */
        virtual void handle_api_status();

        /**
         * @brief Генерация JSON из состояния системы
         * @param state Состояние системы
         * @return JSON строка
         */
        virtual String state_to_json(const settings::moonshine::state_t& state);

        etl::wifi::server_config_t m_config;        ///< Конфигурация (базовый тип)
        ESP8266WebServer m_server;                  ///< Веб-сервер
        settings::moonshine::state_t (*m_state_getter)() = nullptr;  ///< Функция получения состояния
        bool m_initialized = false;                 ///< Флаг инициализации
    };

    // Глобальный тип для удобства
    using MoonshineWebServer = moonshine_web_server;

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
 * webui::moonshine_web_server web_server;
 * webui::moonshine_server_config_t web_config;
 *
 * void setup() {
 *     Serial.begin(115200);
 *
 *     // Настройка конфигурации
 *     web_config.hostname = "moonshine";
 *     web_config.ap_ssid = "Moonshine_AP";
 *     web_config.ap_password = "moonshine123";
 *     web_config.wifi_ssid = "MyWiFi";           // опционально
 *     web_config.wifi_password = "MyPassword";   // опционально
 *
 *     // Установка конфигурации и инициализация
 *     web_server.set_config(web_config);
 *     if (web_server.begin()) {
 *         Serial.println("Web server started");
 *         Serial.print("IP: ");
 *         Serial.println(web_server.get_ip_address());
 *     }
 *
 *     // Установка функции получения состояния
 *     web_server.set_state_getter(get_state);
 * }
 *
 * void loop() {
 *     web_server.handle_client();
 *     // ... остальная логика
 * }
 * @endcode
 */
