#pragma once
// Датчики температуры
#include <Arduino.h>
#include <GyverDS18.h>
#include "etl/etl_optional.h"
#include "etl/etl_array.h"
#include "etl/etl_memory.h"

// для подсчета и калибровки датчиков холла
// пока без нормирования, просто засекаем время и количество срабатываний
struct flow_sensor_t
{
    String name;    // Короткое имя
    String title;   // Полное имя
    uint8_t pin;    // подключение к пину
    volatile int total_count = 0;   // счетчик тиков
    volatile int pulse_count = 0;     // наколенные необработанные тики
    volatile bool is_modified = false; // данные ьыли изменены

    flow_sensor_t(const String& short_name, const String& title_full, uint8_t pin_flow);
    bool init(etl::weak_ptr<flow_sensor_t> instance);
    bool tick();    // проверить накопленные новые срабатывания
    void pulse();   // добавить разабывание датчика по прерыванию
    void reset();   // сбросить все счетчики

    char get_spinner_char(int counter); // возвращает символ для текущего значения
    String get_spinner_state(); // текущее состояние спиннера

    // кабировка счетчика
    struct calibrate_item_t {
        int count = 0;  // значение счетчика 
        unsigned ms = 0;     // в момент времени
    };
    etl::optional<calibrate_item_t> _start;
    etl::optional<calibrate_item_t> _stop;

    bool is_calibrate() const;
    void set_calibrate(bool start);
    void reset_calibrate();
    etl::optional<calibrate_item_t> get_calibrate(); // получить результаты калибровки или текущий накопленный результат
    String format_calibrate_data(etl::optional<calibrate_item_t> value);
};        