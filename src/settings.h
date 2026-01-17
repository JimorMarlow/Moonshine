#pragma once
// Настройки проекта с сохранение в EEPROM или операционную систему

#include <Arduino.h>
namespace settings
{
    const uint16_t temperature_update_delay = 500;  // 500ms
    struct moonshine_t
    {
        // bool    state      = false;    // Велючен свет или нет
        // float   brightness = 1.0;      // Целевой уровень яркости

        void trace() {
            Serial.println("=== moonshine_t settings ===");
            //Serial.printf("state = %s\n", state ? "ON" : "OFF");
            //Serial.printf("brightness = %g\n", brightness);
            Serial.println("========================");
        }            
    };

    String get_uptime_string();
}// settings
