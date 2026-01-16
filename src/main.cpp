#include <Arduino.h>

// Вся конфигурация и платформозависимые настройки в pinout.h
#include "pinout.h"

//////////////////////////////////////////////////////////
#include "etl/etl_memory.h"
#include "etl/etl_optional.h"
#include "etl/etl_vector.h"
#include <GTimer.h>

// Датчики температуры (каждый на отдельный пин для удобства подключения)
#include "sensor_t.h"
//  Температура в нагревательном баке (кубе) с исходным материалом.
auto t_heater_tank = etl::make_shared<temperature_sensor_t>("TANK", "t_heater_tank", T_HEATER_TANK_PIN);  
// Температура воды на выходе из дефлегматора. Показывает эффективность дефлегматора.
auto t_deflegmator_water_out = etl::make_shared<temperature_sensor_t>("DFTR", "temperature_sensor_t", T_DEFLEGMATOR_WATER_OUT_PIN); 
// Температура воды на выходе из охладителя (конденсатора/холодильника). Показывает 
auto t_condenser_water_out = etl::make_shared<temperature_sensor_t>("COOL", "t_condenser_water_out", T_CONDENSER_WATER_OUT_PIN); 

// Все датчики температуры для удобства опроса в цикле
etl::vector<etl::shared_ptr<temperature_sensor_t>> t_sensors  
{
  t_heater_tank, t_deflegmator_water_out, t_condenser_water_out
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
  if (t->sensor.tick()) 
  {
      Serial.print(t->name);
      Serial.print(": ");
      Serial.print(t->sensor.getTemp());
      Serial.print("°C, ");
      Serial.print(t->title);
      Serial.println();
  }
}

String get_uptime_string();

void loop() 
{
  for(auto t : t_sensors)
  {
    check_temperature(t);
  }
}

// Функция возвращает время работы в формате HH:MM:SS
String get_uptime_string() {
  unsigned long currentMillis = millis();
  
  // Вычисляем время в секундах
  unsigned long totalSeconds = currentMillis / 1000;
  
  // Вычисляем часы, минуты, секунды
  unsigned long hours = totalSeconds / 3600;
  unsigned long minutes = (totalSeconds % 3600) / 60;
  unsigned long seconds = totalSeconds % 60;
  
  // Форматируем строку с ведущими нулями
  char buffer[10];
  snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu", hours, minutes, seconds);
  
  return String(buffer);
}