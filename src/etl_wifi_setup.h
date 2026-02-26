#pragma once
/**
 * @file etl_wifi_setup.h
 * @brief Базовая конфигурация WiFi для ESP8266 устройств
 *
 * Платформа: ESP8266 (NodeMCU v3)
 *
 * Особенности:
 * - Базовая структура конфигурации WiFi
 * - Значения по умолчанию для универсального устройства
 * - Наследование для специфичных конфигураций проекта
 */

#include <Arduino.h>

namespace etl
{
    namespace wifi
    {
        /**
         * @brief Базовая конфигурация WiFi сервера
         *
         * Содержит общие параметры для всех устройств:
         * - Имя хоста для mDNS
         * - Параметры точки доступа (AP)
         * - Параметры подключения к внешней сети (STA)
         * - Порт сервера
         * - Интервал обновления данных
         */
        struct server_config_t
        {
            String hostname = "espdevice";              // Имя хоста для mDNS
            String ap_ssid = "ESP_Device_AP";           // SSID точки доступа
            String ap_password = "password123";         // Пароль точки доступа
            String wifi_ssid = "";                      // SSID внешней Wi-Fi сети (пусто = только AP)
            String wifi_password = "password123";       // Пароль внешней Wi-Fi сети
            uint16_t port = 80;                         // Порт веб-сервера
            uint32_t update_interval = 500;             // Интервал обновления данных (мс)
        };

    } // namespace wifi
} // namespace etl
