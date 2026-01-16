#include "sensor_t.h"

temperature_sensor_t::temperature_sensor_t(const String& short_name, const String& title_full, int pin)
: name(short_name)
, title(title_full)
, sensor(pin)
{
}

bool temperature_sensor_t::init()
{
    return sensor.setResolution(12);
}