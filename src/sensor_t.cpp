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

bool temperature_sensor_t::tick()
{
    switch(sensor.tick())
    {
        case DS18_READY:
            value = filter.update(sensor.getTemp());
            return true;
            break;

        case DS18_ERROR:
            if(value)
            {
                value.reset();
                filter.reset();
                return true;
            }
            break; 
    }

    return false;
}

etl::optional<float> temperature_sensor_t::temperature() const
{
    return value;
}