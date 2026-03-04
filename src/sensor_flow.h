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
class flow_sensor_t
{
public:
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

    // Конструктор / деструктор
    flow_sensor_t(const String& short_name, const String& title_full, uint8_t pin_flow);
    virtual ~flow_sensor_t() = default;

    // Инициализация и основные методы
    virtual bool init(etl::weak_ptr<flow_sensor_t> instance);
    virtual bool tick();    // проверить накопленные новые срабатывания
    virtual void pulse();   // добавить разабывание датчика по прерыванию
    virtual void reset();   // сбросить все счетчики

    // Геттеры и сеттеры основных параметров
    const String&   get_name() const { return name; }
    void            set_name(const String& value) { name = value; }

    const String&   get_title() const { return title; }
    void            set_title(const String& value) { title = value; }

    uint8_t         get_pin() const { return pin; }
    void            set_pin(uint8_t value) { pin = value; }

    int  get_total_count() const { return total_count; }
    void set_total_count(int value) { total_count = value; }

    int  get_pulse_count() const { return pulse_count; }
    void set_pulse_count(int value) { pulse_count = value; }

    bool is_data_modified() const { return is_modified; }
    void set_data_modified(bool value) { is_modified = value; }

    // Индикация состояния
    char   get_spinner_char(int counter);
    String get_spinner_state();

    // Параметры датчика
    const sensor_info_t& get_sensor_info() const { return sensor_info; }
    void set_sensor_info(const sensor_info_t& value) { sensor_info = value; }

    // Измерение скорости минутного потока l/min
    uint32_t get_samples_flow_interval() const { return samples_flow_interval; }
    float get_minute_flow_rate() const { return minute_flow_rate; }
    void  set_minute_flow_rate(float value) { minute_flow_rate = value; }

    bool update_minute_flow_rate(int add_pulses);  // Добавить новые отсчеты и рассчитать поток

    /////////////////////////////////////////////////////////////////////
    // Калибровка счетчика
    // используется для изменения значений конкретного экземпляра датчика потока
    // по нажатию кнопки начинают считаться срабатывания и время за которое набирается 1000 мл жидкости
    // после накопления статистики это значение будет установлено как sample_rate - число срабатываний на 1000 мл
    bool is_calibrate() const;
    void set_calibrate(bool start);
    void reset_calibrate();
    etl::optional<sample_t> get_calibrate(); // получить результаты калибровки или текущий накопленный результат
    String format_calibrate_data(etl::optional<sample_t> value);

protected:
    bool was_modified() const { return is_modified;}
    void set_modified(bool enable) {is_modified = enable;}

protected:
    String name;    // Короткое имя
    String title;   // Полное имя
    uint8_t pin;    // подключение к пину
    volatile int total_count = 0;       // счетчик тиков
    volatile int pulse_count = 0;       // наколенные необработанные тики
    volatile bool is_modified = false;  // данные были изменены

    sensor_info_t sensor_info;      // Установить для наследников для правильного расчета показаний счетчика

    // Измерение скорости минутного потока l/min (измеряем количество срабатываний на меньшем диапазоне для более быстрого получения результата)
    const uint32_t samples_flow_interval = 10000; // Интервал измерения (ms) для расчета минутного потока
    etl::queue<sample_t, 1000> samples_flow;   // Набор значений для измерения потока в минуту
    volatile float minute_flow_rate = 0.0;        // Расчтанное мгновенное значение минутного расхода воды l/min

    // Калибровка датчика
    const uint32_t calibrate_valid_interval = 30*1000; // Интервал сохранения результатов калибровки (ms)
    etl::optional<sample_t> _start;
    etl::optional<sample_t> _stop;
};

///////////////////////////////////////////////////////////////////////////////
// Справочник поддерживаемых датчиков с калибровкой или по заводским значениям

///////////////////////////////////////////////////////////////////////////////
// "SEN-HZ06D DC5V Small Size Mini Water Flow Sensor Fluid Flow Meter Switch 0.05-1.5L/min For Coffee Machine"
// https://ali.click/ktwszq
class flow_sensor_SEN_HZ06D : public flow_sensor_t
{
public: 
    flow_sensor_SEN_HZ06D(const String& short_name, const String& title_full, uint8_t pin_flow)
    : flow_sensor_t(short_name, title_full, pin_flow)
    {
        sensor_info = sensor_info_t{"SEN-HZ06D", 4000, etl::Rangef{0.05, 1.5}};
    }
};
///////////////////////////////////////////////////////////////////////////////