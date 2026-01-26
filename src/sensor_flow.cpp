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
    return true;
}

void flow_sensor_t::reset()   // сбросить все счетчики
{
    total_count = 0;   // счетчик тиков
    pulse_count = 0;

    reset_calibrate();
}

bool flow_sensor_t::tick()
{
    if(pulse_count > 0)
    {
        total_count += pulse_count;
        pulse_count = 0;
        return true;
    }
    if(is_modified)
    {
        is_modified = false;
        return true;
    }
    return false;
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
    return _spinner_chars[counter % _spinner_chars.size()];
}

String flow_sensor_t::get_spinner_state() // текущее состояние спиннера
{
    if(is_calibrate()) return String(">");
    return String(get_spinner_char(total_count));
}

void flow_sensor_t::set_calibrate(bool start)
{
    if(start){
        _start = calibrate_item_t{total_count, millis()};
        _stop.reset();
    }
    else{
        _stop = calibrate_item_t{total_count, millis()};
    }
    is_modified = true;
}
bool flow_sensor_t::is_calibrate() const
{
    return _start && !_stop;
}

etl::optional<flow_sensor_t::calibrate_item_t> flow_sensor_t::get_calibrate()
{
    etl::optional<calibrate_item_t> value;
    if(_start)
    {
        if(_stop)
        {
            value = calibrate_item_t{_stop->count - _start->count, _stop->ms - _start->ms};
        }
        else
        {
            value = calibrate_item_t{total_count - _start->count, millis() - _start->ms};
        }
    }
    return value;
}

String flow_sensor_t::format_calibrate_data(etl::optional<flow_sensor_t::calibrate_item_t> value)
{
    if(value)
    {
        // Преобразуем миллисекунды в минуты и секунды
        unsigned long total_seconds = value->ms / 1000;
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