/**
 * @file etl_wifi_setup.cpp
 * @brief Реализация WiFi Setup Server для ESP8266
 *
 * Платформа: ESP8266 (NodeMCU v3)
 */

#include "etl_wifi_setup.h"
#include "etl_wifi_setup_html.h"
#include <LittleFS.h>

namespace etl
{
    namespace wifi
    {
        // Константы для хранения настроек
        static const char* WIFI_SETTINGS_FILE = "/wifi_settings.conf";
        static const uint32_t WIFI_CONNECT_TIMEOUT = 10000;  // 10 секунд

        server_setup::server_setup(const server_config_t& cfg)
            : m_config(cfg)
        {
        }

        server_setup::~server_setup()
        {
            stop();
        }

        bool server_setup::begin()
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
                        start_http_server();
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
                start_http_server();
                return true;
            }

            Serial.println(F("[WiFiSetup] Failed to start AP"));
            return false;
        }

        void server_setup::start_http_server()
        {
            Serial.println(F("[WiFiSetup] Starting HTTP server..."));

            if (m_server != nullptr) {
                delete m_server;
            }

            m_server = new ESP8266WebServer(m_config.port);

            // Настройка роутинга
            setup_http_routes();

            // Запуск сервера
            m_server->begin();
            Serial.print(F("[WiFiSetup] HTTP server started on port "));
            Serial.println(m_config.port);

            // mDNS
            if (MDNS.begin(m_config.hostname.c_str())) {
                Serial.print(F("[WiFiSetup] mDNS: http://"));
                Serial.print(m_config.hostname);
                Serial.println(F(".local"));
                MDNS.addService("http", "tcp", m_config.port);
            } else {
                Serial.println(F("[WiFiSetup] mDNS failed"));
            }
        }

        void server_setup::stop()
        {
            if (!m_initialized) {
                return;
            }

            Serial.println(F("[WiFiSetup] Stopping..."));

            // Остановка mDNS
            MDNS.end();

            // Остановка HTTP сервера
            if (m_server != nullptr) {
                m_server->stop();
                delete m_server;
                m_server = nullptr;
            }

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

        void server_setup::handle()
        {
            if (!m_initialized) {
                return;
            }

            // Обновление статуса подключения
            update_connection_status();
        }

        void server_setup::handle_client()
        {
            if (m_server != nullptr) {
                MDNS.update();
                m_server->handleClient();
            }
        }

        bool server_setup::is_initialized() const
        {
            return m_initialized;
        }

        connection_status_t server_setup::get_connection_status() const
        {
            return m_connection_status;
        }

        bool server_setup::is_connected() const
        {
            return WiFi.status() == WL_CONNECTED;
        }

        String server_setup::get_ip_address() const
        {
            if (WiFi.status() == WL_CONNECTED) {
                return WiFi.localIP().toString();
            }
            return WiFi.softAPIP().toString();
        }

        String server_setup::get_mode() const
        {
            WiFiMode_t mode = WiFi.getMode();
            switch (mode) {
                case WIFI_STA: return "STA";
                case WIFI_AP: return "AP";
                case WIFI_AP_STA: return "AP+STA";
                default: return "UNKNOWN";
            }
        }

        int32_t server_setup::scan_networks(std::vector<scan_result_t>& results)
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

        bool server_setup::connect_to_network(const String& ssid, const String& password, uint32_t timeout)
        {
            Serial.print(F("[WiFiSetup] Connecting to network: "));
            Serial.println(ssid);

            // Сохранение настроек
            m_config.wifi_ssid = ssid;
            m_config.wifi_password = password;

            return connect_to_sta(timeout);
        }

        void server_setup::disconnect()
        {
            Serial.println(F("[WiFiSetup] Disconnecting..."));

            WiFi.disconnect(true);
            m_connection_status = connection_status_t::disconnected;
        }

        bool server_setup::save_settings()
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

        bool server_setup::load_settings()
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

        bool server_setup::reset_settings()
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

        void server_setup::set_config(const server_config_t& cfg)
        {
            m_config = cfg;
        }

        void server_setup::reboot()
        {
            Serial.println(F("[WiFiSetup] Rebooting..."));
            delay(100);
            ESP.reset();
        }

        bool server_setup::start_ap()
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

        bool server_setup::connect_to_sta(uint32_t timeout)
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

        void server_setup::update_connection_status()
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

        String server_setup::get_encryption_type(uint8_t type) const
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

        String server_setup::get_device_icon() const
        {
            if (m_config.device_icon_svg.length() > 0) {
                return m_config.device_icon_svg;
            }

            // Иконка умного устройства по умолчанию
            return F("<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 512 512\"><path d=\"M 256 0 C 114.6 0 0 114.6 0 256 S 114.6 512 256 512 S 512 397.4 512 256 S 397.4 0 256 0 Z M 256 480 C 132.3 480 32 379.7 32 256 S 132.3 32 256 32 S 480 132.3 480 256 S 379.7 480 256 480 Z\" fill=\"#007AFF\"/><path d=\"M 256 128 C 203.1 128 160 171.1 160 224 V 320 C 160 353.1 186.9 380 220 380 H 292 C 325.1 380 352 353.1 352 320 V 224 C 352 171.1 308.9 128 256 128 Z M 320 320 C 320 335.4 307.4 348 292 348 H 220 C 204.6 348 192 335.4 192 320 V 224 C 192 188.8 220.8 160 256 160 S 320 188.8 320 224 V 320 Z\" fill=\"#007AFF\"/><circle cx=\"256\" cy=\"256\" r=\"48\" fill=\"#007AFF\"/></svg>");
        }

        void server_setup::handle_root()
        {
            String html = get_wifi_setup_html();
            m_server->send(200, "text/html", html);
        }

        void server_setup::handle_api_scan()
        {
            Serial.println(F("[WiFiSetup] API: /api/scan"));

            // Проверка кэша
            uint32_t current_time = millis();
            if (m_scan_cache.size() > 0 && (current_time - m_scan_timestamp) < SCAN_CACHE_TIME) {
                Serial.println(F("[WiFiSetup] Returning cached scan results"));
                send_scan_response();
                return;
            }

            // Сканирование сетей
            m_scan_cache.clear();
            scan_networks(m_scan_cache);
            m_scan_timestamp = current_time;

            send_scan_response();
        }

        void server_setup::send_scan_response()
        {
            StaticJsonDocument<2048> doc;
            JsonArray networks = doc.createNestedArray("networks");

            for (const auto& network : m_scan_cache) {
                JsonObject net = networks.createNestedObject();
                net["ssid"] = network.ssid;
                net["rssi"] = network.rssi;
                net["encryption"] = network.encryption;
                net["channel"] = network.channel;
            }

            String response;
            serializeJson(doc, response);
            m_server->send(200, "application/json", response);
        }

        void server_setup::handle_api_connect()
        {
            Serial.println(F("[WiFiSetup] API: /api/connect"));

            if (m_server->hasArg("plain")) {
                String body = m_server->arg("plain");
                StaticJsonDocument<512> doc;
                DeserializationError error = deserializeJson(doc, body);

                if (error) {
                    send_error_response("Invalid JSON");
                    return;
                }

                String ssid = doc["ssid"].as<String>();
                String password = doc["password"].as<String>();

                if (ssid.length() == 0) {
                    send_error_response("SSID is required");
                    return;
                }

                // Подключение к сети
                bool success = connect_to_network(ssid, password);

                if (success) {
                    send_success_response("Connected", get_ip_address());
                } else {
                    send_error_response("Connection failed");
                }
            } else {
                send_error_response("No data provided");
            }
        }

        void server_setup::handle_api_status()
        {
            StaticJsonDocument<512> doc;
            doc["connected"] = is_connected();
            doc["ssid"] = m_config.wifi_ssid;
            doc["ip"] = get_ip_address();
            doc["rssi"] = WiFi.RSSI();
            doc["mode"] = get_mode();

            String response;
            serializeJson(doc, response);
            m_server->send(200, "application/json", response);
        }

        void server_setup::handle_api_save()
        {
            Serial.println(F("[WiFiSetup] API: /api/save"));

            bool success = save_settings();

            if (success) {
                send_success_response("Settings saved");
            } else {
                send_error_response("Failed to save settings");
            }
        }

        void server_setup::handle_api_reset()
        {
            Serial.println(F("[WiFiSetup] API: /api/reset"));

            bool success = reset_settings();

            if (success) {
                send_success_response("Settings reset. Rebooting...");
                delay(1000);
                reboot();
            } else {
                send_error_response("Failed to reset settings");
            }
        }

        void server_setup::handle_api_ap_settings()
        {
            Serial.println(F("[WiFiSetup] API: /api/ap_settings"));

            if (m_server->hasArg("plain")) {
                String body = m_server->arg("plain");
                StaticJsonDocument<512> doc;
                DeserializationError error = deserializeJson(doc, body);

                if (error) {
                    send_error_response("Invalid JSON");
                    return;
                }

                String ap_ssid = doc["ap_ssid"].as<String>();
                String ap_password = doc["ap_password"].as<String>();

                if (ap_ssid.length() == 0) {
                    send_error_response("AP SSID is required");
                    return;
                }

                if (ap_password.length() > 0 && ap_password.length() < 8) {
                    send_error_response("AP password must be at least 8 characters");
                    return;
                }

                // Применение настроек AP
                m_config.ap_ssid = ap_ssid;
                m_config.ap_password = ap_password;

                // Перезапуск точки доступа
                WiFi.softAPdisconnect(true);
                start_ap();

                send_success_response("AP settings applied", m_config.ap_ssid);
            } else {
                send_error_response("No data provided");
            }
        }

        void server_setup::send_success_response(const String& message, const String& extra_data)
        {
            StaticJsonDocument<256> doc;
            doc["success"] = true;
            doc["message"] = message;
            if (extra_data.length() > 0) {
                doc["data"] = extra_data;
            }

            String response;
            serializeJson(doc, response);
            m_server->send(200, "application/json", response);
        }

        void server_setup::send_error_response(const String& message)
        {
            StaticJsonDocument<256> doc;
            doc["success"] = false;
            doc["message"] = message;

            String response;
            serializeJson(doc, response);
            m_server->send(200, "application/json", response);
        }

        String server_setup::get_wifi_setup_html() const
        {
            return get_wifi_setup_html_content(
                m_config.device_name,
                m_config.device_description,
                m_config.device_icon_svg
            );
        }

        void server_setup::setup_http_routes()
        {
            Serial.println(F("[WiFiSetup] Setting up HTTP routes..."));

            // Главная страница
            m_server->on("/", HTTP_GET, [this]() { handle_root(); });

            // API endpoints
            m_server->on("/api/scan", HTTP_GET, [this]() { handle_api_scan(); });
            m_server->on("/api/connect", HTTP_POST, [this]() { handle_api_connect(); });
            m_server->on("/api/status", HTTP_GET, [this]() { handle_api_status(); });
            m_server->on("/api/save", HTTP_POST, [this]() { handle_api_save(); });
            m_server->on("/api/reset", HTTP_POST, [this]() { handle_api_reset(); });
            m_server->on("/api/ap_settings", HTTP_POST, [this]() { handle_api_ap_settings(); });

            // Обработчик для остальных путей - 404
            m_server->onNotFound([this]() {
                m_server->send(404, "text/plain", "Not Found");
            });
        }

    } // namespace wifi
} // namespace etl
