#pragma once
// Датчики температуры
#include <Arduino.h>
#include <GyverDS18.h>
#include "etl/etl_optional.h"
#include "etl/filter/median.h"

struct temperature_sensor_t
{
    String name;    // Короткое имя
    String title;   // Полное имя
    GyverDS18Single sensor;
    etl::optional<float> value; // текущая температура
    etl::filter::median3<float> filter; // сглаживающий фильтр

    temperature_sensor_t(const String& short_name, const String& title_full, int pin);
    bool init();
    bool tick();
    etl::optional<float> temperature() const;
};        