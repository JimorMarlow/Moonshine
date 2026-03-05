#include "sensor_flow.h"

flow_sensor_t::flow_sensor_t(const String& short_name, const String& title_full, uint8_t pin_flow)
: name(short_name)
, title(title_full)
, pin(pin_flow)
{
}

bool flow_sensor_t::init(etl::weak_ptr<flow_sensor_t> instance)
{
    reset();
    pinMode(pin, INPUT_PULLUP); // Настройка пина с подтяжкой к 3.3V
    // attachInterrupt ...

    reset();

    return true;
}

void flow_sensor_t::reset()   // сбросить все счетчики
{
    total_count = 0;   // счетчик тиков
    pulse_count = 0;

    // Размер очереди рассчитывается из максимального потока в минуту
    // отнесенного на интервал измерения
    size_t samble_queue_size = (sensor_info.sample_rate * sensor_info.working_flow_range.high) * (samples_flow_interval / 60000.0);
    samples_flow.reserve(samble_queue_size);
    samples_flow.clear();    // Сброс очереди
    
    reset_calibrate();
}

bool flow_sensor_t::tick()
{
    const int add_pulses = pulse_count; pulse_count = 0;    // time critical place
    if(add_pulses > 0)
    {
        total_count += add_pulses;
        return update_minute_flow_rate(add_pulses);
    }
    if(was_modified())
    {
        set_modified(false);
        return true;
    }
    return update_minute_flow_rate(add_pulses);
}

void flow_sensor_t::pulse()
{
    pulse_count++;
    //   Serial.printf("pulse %s - %s: %d\n", name.c_str(), title.c_str(), total_count);
}

char flow_sensor_t::get_spinner_char(int counter)
{
    // Массив символов для анимации
    static const char _spinner[] = {' ', '-', '/', '|', '+', 'X'};
    static const etl::array _spinner_chars {_spinner};
    // Используем остаток от деления для циклического выбора символа
    // upd. уменьшил на порядок для большей плавности анимации
    return _spinner_chars[(counter/10) % _spinner_chars.size()];
}

String flow_sensor_t::get_spinner_state() // текущее состояние спиннера
{
    if(is_calibrate()) return String(">");
    return String(get_spinner_char(total_count));
}

void flow_sensor_t::set_calibrate(bool start)
{
    if(start){
        _start = sample_t{millis(), total_count};
        _stop.reset();
    }
    else{
        _stop = sample_t{millis(), total_count};
    }
    set_modified(true);
}

bool flow_sensor_t::is_calibrate() const
{
    return _start && !_stop;
}

etl::optional<flow_sensor_t::sample_t> flow_sensor_t::get_calibrate()
{
    etl::optional<sample_t> value;
    if(_start)
    {
        if(_stop)
        {
            if((millis() - _stop->ts) < calibrate_valid_interval)
            {
                value = sample_t{_stop->ts - _start->ts, _stop->count - _start->count};
            }
        }
        else
        {
            value = sample_t{millis() - _start->ts, total_count - _start->count};
        }
    }
    return value;
}

String flow_sensor_t::format_calibrate_data(etl::optional<flow_sensor_t::sample_t> value)
{
    if(value)
    {
        // Преобразуем миллисекунды в минуты и секунды
        unsigned long total_seconds = value->ts / 1000;
        unsigned long minutes = total_seconds / 60;
        unsigned long seconds = total_seconds % 60;

        char buffer[16]; // "MM:SS <count>" где MM до 2 цифр, count до 6 цифр

        // Формат: "MM:SS <count>"
        snprintf(buffer, sizeof(buffer),
                 "%02lu:%02lu %04d",
                 minutes,
                 seconds,
                 value->count);

        return String(buffer);
    }
    return "          ";
}

void flow_sensor_t::reset_calibrate()
{
    _start.reset();
    _stop.reset();
}

// Добавить новые отсчеты и рассчитать поток
bool flow_sensor_t::update_minute_flow_rate(int add_pulses)
{
    bool is_changed = false; // Если есть новые или удалили старые

    auto t_now = millis();
    while(!samples_flow.empty() && (t_now - samples_flow.front().ts) > samples_flow_interval ) {
        samples_flow.pop_front();
        is_changed = true;
    }

    if(add_pulses > 0)
    {
        samples_flow.push(sample_t{t_now, add_pulses});
        is_changed = true;
    }

    if(samples_flow.empty())
    {
        minute_flow_rate = 0.0; // Нет потока воды
    }
    else
    {
        // Считаем количество пульсов за последний интервал и переводим в минутный поток жидкости
        int sum_pulses = 0;
        for(auto value : samples_flow) sum_pulses += value.count;
        float volume_liter = float(sum_pulses) / sensor_info.sample_rate;  // Объем воды за интервал samples_flow_interval
        minute_flow_rate = volume_liter * 60000.0 / samples_flow_interval; // Переводим в минутный объем
    }

    return is_changed;
}
