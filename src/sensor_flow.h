#pragma once
// Датчики температуры
#include <Arduino.h>
#include <GyverDS18.h>
#include "etl/etl_optional.h"
#include "etl/etl_array.h"
#include "etl/etl_memory.h"
#include "etl/etl_utility.h"

// для подсчета и калибровки датчиков холла
// пока без нормирования, просто засекаем время и количество срабатываний
struct flow_sensor_t
{
    String name;    // Короткое имя
    String title;   // Полное имя
    uint8_t pin;    // подключение к пину
    volatile int total_count = 0;       // счетчик тиков
    volatile int pulse_count = 0;       // наколенные необработанные тики
    volatile bool is_modified = false;  // данные были изменены

    flow_sensor_t(const String& short_name, const String& title_full, uint8_t pin_flow);
    bool init(etl::weak_ptr<flow_sensor_t> instance);
    bool tick();    // проверить накопленные новые срабатывания
    void pulse();   // добавить разабывание датчика по прерыванию
    void reset();   // сбросить все счетчики

    char get_spinner_char(int counter); // возвращает символ для текущего значения
    String get_spinner_state(); // текущее состояние спиннера

    // Показания счетчика с привязкой по времени
    struct sample_t {
        int count = 0;          // значение счетчика (или количество изменений от прошлого раза)
        unsigned ms = 0;        // в момент времени
    };

    // Значения, которые зависят от датчика
    struct sensor_info_t {
        String  sensor_name = "flow_sensor";    // Название датчика
        int     sample_rate = 1000;             // количество срабатываний на 1000 мл жидскости
        etl::range_t working_flow_range {0.2, 1.5}; // Поток 0.2 - 1.5 литра в миниту
    }
    sensor_info_t sensor_info;      // Установить для наследников для правильно расчета показаний счетчика

    /////////////////////////////////////////////////////////////////////
    // Кабировка счетчика    
    // используется для изменения значений конкретного экземпляра датчика потока 
    // по нажатию кнопки начинают считаться срабатывания и время за которое набирается 1000 мл жидкости
    // после накопления статистики это значение будет установлено как sample_rate - число срабатываний на 1000 мл
    etl::optional<sample_t> _start;
    etl::optional<sample_t> _stop;

    bool is_calibrate() const;
    void set_calibrate(bool start);
    void reset_calibrate();
    etl::optional<sample_t> get_calibrate(); // получить результаты калибровки или текущий накопленный результат
    String format_calibrate_data(etl::optional<sample_t> value);
};        