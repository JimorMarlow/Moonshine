#pragma once
// Настройки проекта с сохранение в EEPROM или операционную систему

#include <Arduino.h>
#include "etl/etl_optional.h"
namespace settings
{
namespace moonshine 
{
    const uint16_t temperature_update_delay = 500;  // 500ms
    struct state_t
    {
        uint32_t             uptime_ms = 0;                // Время от начала работы
        etl::optional<float> steam_temperature = 0.0;      // Температура °C пара в верхней части колонных
        etl::optional<float> heater_temperature = 0.0;     // Температура °C в нагревательном баке
        etl::optional<float> deflegmater_temperature = 0.0;// Температура °C воды на выходе дефлегматора
        etl::optional<float> condenser_temperature = 0.0;  // Температура °C воды на выходе охладителя

        float deflegmater_water_flow = 0.0; // Скорость потока воды ml/min на выходе дефлегматора
        float condenser_water_flow = 0.0;   // Скорость потока воды ml/min на выходе охладителя

        void trace() {
            Serial.println("=== settings::moonshine::state_t ===");
            Serial.printf("uptime_ms = %d\n", uptime_ms);
            Serial.printf("steam_temperature = %s\n", steam_temperature ? String(*steam_temperature).c_str() : "---");
            Serial.printf("heater_temperature = %s\n", heater_temperature ? String(*heater_temperature).c_str() : "---");
            Serial.printf("deflegmater_temperature = %s\n", deflegmater_temperature ? String(*deflegmater_temperature).c_str() : "---");
            Serial.printf("condenser_temperature = %s\n", condenser_temperature ? String(*condenser_temperature).c_str() : "---");
            Serial.printf("deflegmater_water_flow = %g\n", deflegmater_water_flow);
            Serial.printf("condenser_water_flow = %g\n", condenser_water_flow);
            Serial.println("========================");
        }            
    };

    String get_uptime_string();
}// moonshine
}// settings
