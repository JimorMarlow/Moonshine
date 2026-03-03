#pragma once
// Датчики температуры
#include <Arduino.h>
#include <GyverDS18.h>
#include "etl/etl_optional.h"
#include "etl/etl_array.h"
#include "etl/etl_memory.h"
#include "etl/etl_utility.h"
#include "etl/etl_queue.h"

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
        uint32_t    ts = 0;        // в момент времени (ms)
        int         count = 0;     // значение счетчика (или количество изменений от прошлого раза)
    };

    // Значения, которые зависят от датчика
    struct sensor_info_t {
        String  sensor_name = "flow_sensor";    // Название датчика
        int     sample_rate = 1000;             // количество срабатываний на 1000 мл жидскости
        etl::Rangef working_flow_range = etl::Rangef(1.0, 30.0); // Поток 1.0 - 30.0 литра в миниту
    };
    sensor_info_t sensor_info;      // Установить для наследников для правильного расчета показаний счетчика

    // Измерение скорости минутного потока l/min (измеряем количество срабатываний на меньшем диапазоне для более быстрого получения результата)
    const uint32_t samples_flow_interval = 10000; // Интервал измерения (ms) для расчета минутного потока
    etl::queue<sample_t, 1000> samples_flow;   // Набор значений для измерения потока в минуту
    volatile float minute_flow_rate = 0.0;        // Расчтанное мгновенное значение минутного расхода воды l/min  

    bool update_minute_flow_rate(int add_pulses);  // Добавить новые отсчеты и рассчитать поток, true - данные были изменены 
    float get_minute_flow_rate() const { return minute_flow_rate; } // Мгновенный поток жидкости [l/min]

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

///////////////////////////////////////////////////////////////////////////////
// Справочник поддерживаемых датчиков с калибровкой или по заводским значениям

///////////////////////////////////////////////////////////////////////////////
// "SEN-HZ06D DC5V Small Size Mini Water Flow Sensor Fluid Flow Meter Switch 0.05-1.5L/min For Coffee Machine"
// https://ali.click/ktwszq
struct flow_sensor_SEN_HZ06D : public flow_sensor_t
{
    flow_sensor_SEN_HZ06D(const String& short_name, const String& title_full, uint8_t pin_flow)
    : flow_sensor_t(short_name, title_full, pin_flow)
    {
        sensor_info = sensor_info_t{"SEN-HZ06D", 4000, etl::Rangef{0.05, 1.5}};
    }
};
///////////////////////////////////////////////////////////////////////////////