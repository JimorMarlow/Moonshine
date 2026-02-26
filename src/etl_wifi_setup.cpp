/**
 * @file etl_wifi_setup.cpp
 * @brief Реализация WiFi Setup Server для ESP8266
 *
 * Платформа: ESP8266 (NodeMCU v3)
 */

#include "etl_wifi_setup.h"
#include <LittleFS.h>

namespace etl
{
    namespace wifi
    {
        // Константы для хранения настроек
        static const char* WIFI_SETTINGS_FILE = "/wifi_settings.conf";
        static const uint32_t WIFI_CONNECT_TIMEOUT = 10000;  // 10 секунд

        wifi_setup::wifi_setup(const server_config_t& cfg)
            : m_config(cfg)
        {
        }

        wifi_setup::~wifi_setup()
        {
            stop();
        }

        bool wifi_setup::begin()
        {
            Serial.println(F("[WiFiSetup] Initializing..."));

            // Попытка загрузки сохранённых настроек
            if (load_settings()) {
                Serial.println(F("[WiFiSetup] Loaded saved settings"));

                // Если есть сохранённые настройки, пробуем подключиться
                if (m_config.wifi_ssid.length() > 0) {
                    Serial.print(F("[WiFiSetup] Connecting to saved network: "));
                    Serial.println(m_config.wifi_ssid);

                    if (connect_to_sta(WIFI_CONNECT_TIMEOUT)) {
                        Serial.println(F("[WiFiSetup] Connected to saved network"));
                        m_initialized = true;
                        return true;
                    } else {
                        Serial.println(F("[WiFiSetup] Failed to connect to saved network"));
                    }
                }
            }

            // Запуск в режиме точки доступа
            Serial.println(F("[WiFiSetup] Starting AP mode..."));
            if (start_ap()) {
                Serial.println(F("[WiFiSetup] AP started successfully"));
                m_initialized = true;
                return true;
            }

            Serial.println(F("[WiFiSetup] Failed to start AP"));
            return false;
        }

        void wifi_setup::stop()
        {
            if (!m_initialized) {
                return;
            }

            Serial.println(F("[WiFiSetup] Stopping..."));

            // Отключение от WiFi
            WiFi.disconnect(true);

            // Остановка точки доступа
            WiFi.softAPdisconnect(true);

            // Отключение WiFi
            WiFi.mode(WIFI_OFF);

            m_initialized = false;
            m_connection_status = connection_status_t::disconnected;

            Serial.println(F("[WiFiSetup] Stopped"));
        }

        void wifi_setup::handle()
        {
            if (!m_initialized) {
                return;
            }

            // Обновление статуса подключения
            update_connection_status();
        }

        bool wifi_setup::is_initialized() const
        {
            return m_initialized;
        }

        connection_status_t wifi_setup::get_connection_status() const
        {
            return m_connection_status;
        }

        bool wifi_setup::is_connected() const
        {
            return WiFi.status() == WL_CONNECTED;
        }

        String wifi_setup::get_ip_address() const
        {
            if (WiFi.status() == WL_CONNECTED) {
                return WiFi.localIP().toString();
            }
            return WiFi.softAPIP().toString();
        }

        String wifi_setup::get_mode() const
        {
            WiFiMode_t mode = WiFi.getMode();
            switch (mode) {
                case WIFI_STA: return "STA";
                case WIFI_AP: return "AP";
                case WIFI_AP_STA: return "AP+STA";
                default: return "UNKNOWN";
            }
        }

        int32_t wifi_setup::scan_networks(std::vector<scan_result_t>& results)
        {
            Serial.println(F("[WiFiSetup] Scanning networks..."));

            results.clear();

            // Переключение в режим STA для сканирования
            WiFiMode_t current_mode = WiFi.getMode();
            WiFi.mode(WIFI_STA);

            int32_t count = WiFi.scanNetworks();

            // Возврат предыдущего режима
            if (current_mode == WIFI_AP) {
                WiFi.mode(WIFI_AP);
            } else if (current_mode == WIFI_AP_STA) {
                WiFi.mode(WIFI_AP_STA);
            }

            if (count == 0) {
                Serial.println(F("[WiFiSetup] No networks found"));
                return 0;
            }

            Serial.printf("[WiFiSetup] Found %d networks\n", count);

            for (int32_t i = 0; i < count; ++i) {
                scan_result_t result;
                result.ssid = WiFi.SSID(i);
                result.rssi = WiFi.RSSI(i);
                result.encryption = get_encryption_type(WiFi.encryptionType(i));
                result.channel = WiFi.channel(i);

                results.push_back(result);

                Serial.printf("[WiFiSetup] Network %d: %s (RSSI: %d, Encryption: %s)\n",
                              i + 1, result.ssid.c_str(), result.rssi, result.encryption.c_str());
            }

            // Сортировка по уровню сигнала (убывание)
            std::sort(results.begin(), results.end(),
                      [](const scan_result_t& a, const scan_result_t& b) {
                          return a.rssi > b.rssi;
                      });

            WiFi.scanDelete();
            return count;
        }

        bool wifi_setup::connect_to_network(const String& ssid, const String& password, uint32_t timeout)
        {
            Serial.print(F("[WiFiSetup] Connecting to network: "));
            Serial.println(ssid);

            // Сохранение настроек
            m_config.wifi_ssid = ssid;
            m_config.wifi_password = password;

            return connect_to_sta(timeout);
        }

        void wifi_setup::disconnect()
        {
            Serial.println(F("[WiFiSetup] Disconnecting..."));

            WiFi.disconnect(true);
            m_connection_status = connection_status_t::disconnected;
        }

        bool wifi_setup::save_settings()
        {
            Serial.println(F("[WiFiSetup] Saving settings..."));

            if (!LittleFS.begin()) {
                Serial.println(F("[WiFiSetup] Failed to mount LittleFS"));
                return false;
            }

            File file = LittleFS.open(WIFI_SETTINGS_FILE, "w");
            if (!file) {
                Serial.println(F("[WiFiSetup] Failed to open settings file for writing"));
                LittleFS.end();
                return false;
            }

            // Запись настроек в файл
            file.println(m_config.hostname);
            file.println(m_config.ap_ssid);
            file.println(m_config.ap_password);
            file.println(m_config.wifi_ssid);
            file.println(m_config.wifi_password);
            file.println(m_config.port);
            file.println(m_config.update_interval);

            file.close();
            LittleFS.end();

            Serial.println(F("[WiFiSetup] Settings saved"));
            return true;
        }

        bool wifi_setup::load_settings()
        {
            if (!LittleFS.begin()) {
                Serial.println(F("[WiFiSetup] Failed to mount LittleFS"));
                return false;
            }

            File file = LittleFS.open(WIFI_SETTINGS_FILE, "r");
            if (!file) {
                Serial.println(F("[WiFiSetup] Settings file not found"));
                LittleFS.end();
                return false;
            }

            // Чтение настроек из файла
            String hostname = file.readStringUntil('\n');
            String ap_ssid = file.readStringUntil('\n');
            String ap_password = file.readStringUntil('\n');
            String wifi_ssid = file.readStringUntil('\n');
            String wifi_password = file.readStringUntil('\n');
            String port_str = file.readStringUntil('\n');
            String interval_str = file.readStringUntil('\n');

            file.close();
            LittleFS.end();

            // Удаление символов перевода строки
            hostname.trim();
            ap_ssid.trim();
            ap_password.trim();
            wifi_ssid.trim();
            wifi_password.trim();
            port_str.trim();
            interval_str.trim();

            // Проверка на пустые значения
            if (hostname.length() == 0) {
                Serial.println(F("[WiFiSetup] Invalid settings file"));
                return false;
            }

            // Применение настроек
            m_config.hostname = hostname;
            m_config.ap_ssid = ap_ssid;
            m_config.ap_password = ap_password;
            m_config.wifi_ssid = wifi_ssid;
            m_config.wifi_password = wifi_password;

            if (port_str.length() > 0) {
                m_config.port = port_str.toInt();
            }

            if (interval_str.length() > 0) {
                m_config.update_interval = interval_str.toInt();
            }

            Serial.println(F("[WiFiSetup] Settings loaded"));
            return true;
        }

        bool wifi_setup::reset_settings()
        {
            Serial.println(F("[WiFiSetup] Resetting settings..."));

            if (!LittleFS.begin()) {
                Serial.println(F("[WiFiSetup] Failed to mount LittleFS"));
                return false;
            }

            if (LittleFS.exists(WIFI_SETTINGS_FILE)) {
                if (!LittleFS.remove(WIFI_SETTINGS_FILE)) {
                    Serial.println(F("[WiFiSetup] Failed to remove settings file"));
                    LittleFS.end();
                    return false;
                }
            }

            LittleFS.end();

            // Сброс конфигурации к значениям по умолчанию
            m_config = server_config_t();

            Serial.println(F("[WiFiSetup] Settings reset"));
            return true;
        }

        void wifi_setup::set_config(const server_config_t& cfg)
        {
            m_config = cfg;
        }

        void wifi_setup::reboot()
        {
            Serial.println(F("[WiFiSetup] Rebooting..."));
            delay(100);
            ESP.reset();
        }

        bool wifi_setup::start_ap()
        {
            Serial.print(F("[WiFiSetup] Starting AP: "));
            Serial.println(m_config.ap_ssid);

            // Установка режима AP
            WiFi.mode(WIFI_AP);

            // Запуск точки доступа
            if (!WiFi.softAP(m_config.ap_ssid.c_str(), m_config.ap_password.c_str())) {
                Serial.println(F("[WiFiSetup] Failed to start AP"));
                return false;
            }

            Serial.print(F("[WiFiSetup] AP IP address: "));
            Serial.println(WiFi.softAPIP());

            m_connection_status = connection_status_t::disconnected;
            return true;
        }

        bool wifi_setup::connect_to_sta(uint32_t timeout)
        {
            if (m_config.wifi_ssid.length() == 0) {
                Serial.println(F("[WiFiSetup] No SSID configured"));
                return false;
            }

            Serial.print(F("[WiFiSetup] Connecting to "));
            Serial.println(m_config.wifi_ssid);

            // Установка режима STA
            WiFi.mode(WIFI_STA);

            // Подключение к сети
            WiFi.begin(m_config.wifi_ssid.c_str(), m_config.wifi_password.c_str());

            // Ожидание подключения
            uint32_t start_time = millis();
            while (WiFi.status() != WL_CONNECTED && (millis() - start_time) < timeout) {
                delay(500);
                Serial.print(F("."));
            }

            if (WiFi.status() == WL_CONNECTED) {
                Serial.println(F("\n[WiFiSetup] Connected"));
                Serial.print(F("[WiFiSetup] IP address: "));
                Serial.println(WiFi.localIP());

                m_connection_status = connection_status_t::connected;
                return true;
            }

            Serial.println(F("\n[WiFiSetup] Connection timeout"));
            m_connection_status = connection_status_t::error;

            // Возврат в режим AP при ошибке подключения
            WiFi.mode(WIFI_AP);
            WiFi.softAP(m_config.ap_ssid.c_str(), m_config.ap_password.c_str());

            return false;
        }

        void wifi_setup::update_connection_status()
        {
            wl_status_t status = WiFi.status();

            switch (status) {
                case WL_CONNECTED:
                    m_connection_status = connection_status_t::connected;
                    break;

                case WL_DISCONNECTED:
                case WL_IDLE_STATUS:
                    m_connection_status = connection_status_t::disconnected;
                    break;

                case WL_CONNECT_FAILED:
                case WL_CONNECTION_LOST:
                    m_connection_status = connection_status_t::error;
                    break;

                default:
                    break;
            }
        }

        String wifi_setup::get_encryption_type(uint8_t type) const
        {
            switch (type) {
                case ENC_TYPE_NONE:
                    return "Open";
                case ENC_TYPE_WEP:
                    return "WEP";
                case ENC_TYPE_TKIP:
                    return "WPA";
                case ENC_TYPE_CCMP:
                    return "WPA2";
                case ENC_TYPE_AUTO:
                    return "WPA/WPA2";
                default:
                    return "Unknown";
            }
        }

    } // namespace wifi
} // namespace etl
