#include <Arduino.h>

// Вся конфигурация и платформозависимые настройки в pinout.h
#include "pinout.h"
#include "settings.h"

//////////////////////////////////////////////////////////
#include "etl/etl_memory.h"
#include "etl/etl_optional.h"
#include "etl/etl_vector.h"
#include <GTimer.h>

// Датчики температуры (каждый на отдельный пин для удобства подключения)
#include "sensor_t.h"
//  Температура в нагревательном баке (кубе) с исходным материалом.
auto t_heater_tank = etl::make_shared<temperature_sensor_t>("H", "t_heater_tank", T_HEATER_TANK_PIN);  
// Температура воды на выходе из дефлегматора. Показывает эффективность дефлегматора.
auto t_deflegmator_water_out = etl::make_shared<temperature_sensor_t>("D", "t_deflegmator_water_out", T_DEFLEGMATOR_WATER_OUT_PIN); 
// Температура воды на выходе из охладителя (конденсатора/холодильника). Показывает 
auto t_condenser_water_out = etl::make_shared<temperature_sensor_t>("C", "t_condenser_water_out", T_CONDENSER_WATER_OUT_PIN); 
// Температура верхней части колонны
auto t_top_column  = etl::make_shared<temperature_sensor_t>("S", "t_top_column", T_TOP_COLUMN_PIN); 

// Все датчики температуры для удобства опроса в цикле
etl::vector<etl::shared_ptr<temperature_sensor_t>> t_sensors  
{
  t_top_column, t_deflegmator_water_out, t_condenser_water_out, t_heater_tank
};

void setup() {
    Serial.begin(115200);
    if(SERIAL_INIT_DELAY > 0) delay(SERIAL_INIT_DELAY);  // для ESP32 C3 supermini нуждо сделать задержку, чтобы выводилась отладочная информация
    
    Serial.printf("Moonshine v%s started...\n", String(MS_VERSION_STRING).c_str());
    // установка разрешения температурных датчиков. Влияет на период опроса
    for(auto t : t_sensors)
    {
      Serial.printf("Init %s (%s)... %s\n", t->name.c_str(), t->title.c_str(), t->init() ? "OK" : "ERROR");
    }
}

void check_temperature(etl::shared_ptr<temperature_sensor_t> t)
{
  if (t->tick()) 
  {
      Serial.print(settings::get_uptime_string());
      Serial.print(" ");
      Serial.print(t->name);
      Serial.print(": ");
      if(auto value = t->temperature(); value)
      {
        Serial.print(*value, 2);
        Serial.print("°C, ");
      }
      else{
        Serial.print("--- ");
      }   
      Serial.print(t->title);
      Serial.println();
  }
}

void loop() 
{
  for(auto t : t_sensors)
  {
    check_temperature(t);
  }
//  delay(1000);
}