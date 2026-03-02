#pragma once
/**
 * @file etl_wifi_setup.h
 * @brief WiFi Setup Server для первичной настройки WiFi подключения
 *
 * Платформа: ESP8266 (NodeMCU v3)
 *
 * Особенности:
 * - Режим точки доступа для настройки WiFi
 * - Сканирование доступных сетей
 * - Подключение к выбранной сети
 * - Сохранение настроек в энергонезависимой памяти
 * - Сброс к заводским настройкам
 *
 * @note Класс предоставляет только серверную часть для настройки WiFi.
 *       Веб-интерфейс должен быть реализован отдельно.
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

namespace etl
{
    namespace wifi
    {
        /**
         * @brief Конфигурация WiFi сервера
         *
         * Содержит параметры для точки доступа и внешней сети.
         */
        struct server_config_t
        {
            String hostname = "espdevice";              // Имя хоста для mDNS
            String ap_ssid = "ESP_Device_AP";           // SSID точки доступа
            String ap_password = "password123";         // Пароль точки доступа
            String wifi_ssid = "";                       // SSID внешней сети (пусто = не подключено)
            String wifi_password = "";                   // Пароль внешней сети
            uint16_t port = 80;                         // Порт веб-сервера
            uint32_t update_interval = 500;             // Интервал обновления данных (мс)
        };

        /**
         * @brief Результат сканирования WiFi сети
         */
        struct scan_result_t
        {
            String ssid;                                // SSID сети
            int32_t rssi;                               // Уровень сигнала (dBm)
            String encryption;                          // Тип шифрования (WPA2, WPA, Open)
            uint8_t channel;                            // Канал
        };

        /**
         * @brief Статус подключения WiFi
         */
        enum class connection_status_t
        {
            disconnected,                               // Не подключено
            connecting,                                 // Подключение
            connected,                                  // Подключено
            error                                       // Ошибка
        };

        /**
         * @brief Класс WiFi Setup Server
         *
         * Предоставляет функционал для первичной настройки WiFi подключения.
         * Работает в режиме точки доступа или подключается к внешней сети.
         */
        class server_setup
        {
        public:
            /**
             * @brief Конструктор
             * @param cfg Конфигурация WiFi сервера
             */
            explicit server_setup(const server_config_t& cfg = server_config_t());

            /**
             * @brief Деструктор
             *
             * Виртуальный деструктор для корректного наследования.
             * Вызывает stop() для освобождения ресурсов.
             */
            virtual ~server_setup();

            /**
             * @brief Инициализация WiFi сервера
             *
             * - Запуск в режиме точки доступа
             * - Настройка сети
             *
             * @return true при успешной инициализации
             */
            virtual bool begin();

            /**
             * @brief Остановка WiFi сервера
             *
             * - Отключение от WiFi сети
             * - Остановка точки доступа
             * - Сброс флага инициализации
             */
            virtual void stop();

            /**
             * @brief Основной цикл обработки
             *
             * Вызывать регулярно из loop() для обработки событий WiFi
             */
            virtual void handle();

            /**
             * @brief Проверка инициализации
             * @return true если сервер инициализирован
             */
            virtual bool is_initialized() const;

            /**
             * @brief Получить статус подключения
             * @return Статус подключения
             */
            virtual connection_status_t get_connection_status() const;

            /**
             * @brief Проверка подключения к WiFi сети
             * @return true если подключено к внешней сети
             */
            virtual bool is_connected() const;

            /**
             * @brief Получить IP адрес
             * @return IP адрес в формате String
             */
            virtual String get_ip_address() const;

            /**
             * @brief Получить режим работы
             * @return "AP" если точка доступа, "STA" если клиент, "AP+STA" если оба режима
             */
            virtual String get_mode() const;

            /**
             * @brief Сканирование доступных WiFi сетей
             * @param results Вектор для результатов сканирования
             * @return Количество найденных сетей
             */
            virtual int32_t scan_networks(std::vector<scan_result_t>& results);

            /**
             * @brief Подключение к WiFi сети
             * @param ssid SSID сети
             * @param password Пароль сети
             * @param timeout Таймаут подключения (мс, по умолчанию 10000)
             * @return true при успешном подключении
             */
            virtual bool connect_to_network(const String& ssid, const String& password, uint32_t timeout = 10000);

            /**
             * @brief Отключение от WiFi сети
             */
            virtual void disconnect();

            /**
             * @brief Сохранение настроек WiFi
             * @return true при успешном сохранении
             */
            virtual bool save_settings();

            /**
             * @brief Загрузка сохранённых настроек WiFi
             * @return true если настройки загружены успешно
             */
            virtual bool load_settings();

            /**
             * @brief Сброс настроек WiFi к заводским
             * @return true при успешном сбросе
             */
            virtual bool reset_settings();

            /**
             * @brief Установить конфигурацию сервера
             * @param cfg Новая конфигурация
             * @note Должно быть вызвано до begin() или после stop()
             */
            virtual void set_config(const server_config_t& cfg);

            /**
             * @brief Получить текущую конфигурацию
             * @return Конфигурация сервера
             */
            virtual const server_config_t& get_config() const { return m_config; }

            /**
             * @brief Перезагрузка устройства
             * @note Вызывает ESP.reset()
             */
            virtual void reboot();

        protected:
            server_config_t m_config;                   ///< Конфигурация
            bool m_initialized = false;                 ///< Флаг инициализации
            connection_status_t m_connection_status = connection_status_t::disconnected;  ///< Статус подключения

            /**
             * @brief Запуск точки доступа
             * @return true при успешном запуске
             */
            virtual bool start_ap();

            /**
             * @brief Подключение к внешней сети
             * @param timeout Таймаут подключения (мс)
             * @return true при успешном подключении
             */
            virtual bool connect_to_sta(uint32_t timeout);

            /**
             * @brief Обновление статуса подключения
             */
            virtual void update_connection_status();

            /**
             * @brief Получить тип шифрования из WiFi.encryptionType()
             * @param type Тип шифрования
             * @return Строковое представление типа шифрования
             */
            virtual String get_encryption_type(uint8_t type) const;
        };

    } // namespace wifi
} // namespace etl

/**
 * @brief Пример использования в main.cpp:
 *
 * @code
 * #include "etl_wifi_setup.h"
 *
 * // Глобальный экземпляр
 * etl::wifi::server_setup wifi_server;
 * etl::wifi::server_config_t wifi_config;
 *
 * void setup() {
 *     Serial.begin(115200);
 *
 *     // Настройка конфигурации
 *     wifi_config.hostname = "moonshine";
 *     wifi_config.ap_ssid = "Moonshine_AP";
 *     wifi_config.ap_password = "moonshine123";
 *
 *     // Попытка загрузки сохранённых настроек
 *     if (wifi_server.load_settings()) {
 *         // Настройки загружены, пробуем подключиться
 *         wifi_server.set_config(wifi_config);
 *     } else {
 *         // Настроек нет, используем конфигурацию по умолчанию
 *         wifi_server.set_config(wifi_config);
 *     }
 *
 *     // Инициализация WiFi сервера
 *     if (wifi_server.begin()) {
 *         Serial.println("WiFi setup server started");
 *         Serial.print("IP: ");
 *         Serial.println(wifi_server.get_ip_address());
 *     }
 * }
 *
 * void loop() {
 *     // Обработка событий WiFi
 *     wifi_server.handle();
 *
 *     // Проверка статуса подключения
 *     if (wifi_server.is_connected()) {
 *         Serial.println("Connected to WiFi");
 *         Serial.print("IP: ");
 *         Serial.println(wifi_server.get_ip_address());
 *     }
 *
 *     // ... остальная логика
 * }
 *
 * // Пример сканирования сетей
 * void scan_wifi_networks() {
 *     std::vector<etl::wifi::scan_result_t> networks;
 *     int32_t count = wifi_server.scan_networks(networks);
 *
 *     Serial.printf("Found %d networks\n", count);
 *     for (const auto& network : networks) {
 *         Serial.printf("SSID: %s, RSSI: %d, Encryption: %s\n",
 *                       network.ssid.c_str(), network.rssi, network.encryption.c_str());
 *     }
 * }
 *
 * // Пример подключения к сети
 * void connect_to_wifi() {
 *     if (wifi_server.connect_to_network("MyWiFi", "mypassword")) {
 *         Serial.println("Connected successfully");
 *         wifi_server.save_settings();  // Сохранить настройки
 *     } else {
 *         Serial.println("Connection failed");
 *     }
 * }
 *
 * // Пример сброса настроек
 * void factory_reset() {
 *     wifi_server.reset_settings();
 *     wifi_server.reboot();  // Перезагрузка для применения сброса
 * }
 * @endcode
 */
