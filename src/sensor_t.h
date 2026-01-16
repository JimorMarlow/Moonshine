#pragma once
// Датчики температуры
#include <Arduino.h>
#include <GyverDS18.h>

struct temperature_sensor_t
{
    String name;    // Короткое имя
    String title;   // Полное имя
    GyverDS18Single sensor;

    temperature_sensor_t(const String& short_name, const String& title_full, int pin);
    bool init();
};        