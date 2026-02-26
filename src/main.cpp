#include <Arduino.h>

// Вся конфигурация и платформозависимые настройки в pinout.h
#include "pinout.h"
#include "settings.h"
#include "etl/etl_utility.h"

// Глобальные данные
settings::moonshine::state_t moonshine_state; // текущее состояние системы дистилляции

// получить текущее состояние системы
settings::moonshine::state_t get_moonshine_state() {
  return moonshine_state;
}

void update_moonshine_state(const settings::moonshine::state_t& state) {
  moonshine_state = state;
  moonshine_state.uptime_ms = millis(); // Обновляем текущее время установки актуального состояния
}

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

// Датчики потока воды
#include "sensor_flow.h"
// Поток воды на выходе из дефлегматора. Нужен для контроля температуры по расходу воды
etl::shared_ptr<flow_sensor_t> flow_deflegmator = etl::make_shared<flow_sensor_t>("D", "flow_deflegmator", F_DEFLEGMATOR_WATER_PIN); 
// Поток воды на выходе из охладителя (конденсатора/холодильника). Нужен для контроля температуры по расходу воды
etl::shared_ptr<flow_sensor_t> flow_condenser = etl::make_shared<flow_sensor_t>("C", "flow_condenser", F_CONDENSER_WATER_PIN); 
etl::vector<etl::shared_ptr<flow_sensor_t>> f_sensors  
{
  flow_deflegmator, flow_condenser
};

// ===== ОБРАБОТЧИК ПРЕРЫВАНИЙ =====
void IRAM_ATTR pulse_counter_D() {
    if(auto def = flow_deflegmator; def) {
      def->pulse();
    }
}
void IRAM_ATTR pulse_counter_C() {
    if(auto con = flow_condenser; con) {
      con->pulse();
    }
}
///////////////////////////////////

// Дисплей LED 2004 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);  // Address, columns, rows [web:8]

#include "EncButton.h"
etl::shared_ptr<Button> btn = etl::make_shared<Button>(TOUCH_BTN_PIN);
volatile int btn_clicks = 0;  // для проверки нажатий кнопки


// WEB-UI
#include "web-ui.h"
// Глобальный экземпляр веб-сервера
etl::unique_ptr<webui::MoonshineWebServer> web_server;
bool use_web_ui = true;

///////////////////////////////////////////
void setup() {
    Serial.begin(115200);
    if(SERIAL_INIT_DELAY > 0) delay(SERIAL_INIT_DELAY);  // для ESP32 C3 supermini нуждо сделать задержку, чтобы выводилась отладочная информация
    
    // Display
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.printf("Moonshine v%s", String(MS_VERSION_STRING).c_str());
    delay(1000);
    lcd.clear();

    Serial.printf("\nMoonshine v%s started...\n", String(MS_VERSION_STRING).c_str());
    // установка разрешения температурных датчиков. Влияет на период опроса
    for(auto t : t_sensors)
    {
      if(t)
      {
        Serial.printf("Init %s (%s)... ", t->name.c_str(), t->title.c_str());
        String is_init = t->init() ? "OK" : "FAIL";
        Serial.println(is_init.c_str());
      }
    }

    // Настройка датчиков холла для измерения потока воды
    if(flow_deflegmator && flow_deflegmator->init(flow_deflegmator)) {
      attachInterrupt(digitalPinToInterrupt(flow_deflegmator->pin), pulse_counter_D, FALLING);
    }

    if(flow_condenser && flow_condenser->init(flow_condenser)) {
      attachInterrupt(digitalPinToInterrupt(flow_condenser->pin), pulse_counter_C, FALLING);
    }

    ////////////////////////////////////////////////////////////////////////////
    // Button. Если зажата при включении - сбросить настройки сети к заводским
    if (btn && btn->read()) Serial.println("Кнопка нажата при старте");
    // или так
    bool is_btn_hold_on_start = false;
    if(btn)
    {
      do {
        btn->tick();
        if (btn->hold()) { is_btn_hold_on_start = true; }
      } while (btn->busy() && !is_btn_hold_on_start);
    }
    Serial.println(is_btn_hold_on_start ? "is_btn_hold_on_start = true" : "is_btn_hold_on_start = false");

    ////////////////////////////////////////////////////////////////////////////
    // WEB-UI
    if(use_web_ui)
    {
      webui::moonshine_server_config_t web_config;

      // Настройка конфигурации веб-сервера
      web_config.hostname = "moonshine";
      web_config.ap_ssid = "Moonshine_AP";
      web_config.ap_password = "moonshine123";
      web_config.port = 80;
      web_config.update_interval = 500; // 500ms

      // Раскомментировать для подключения к Wi-Fi сети
      // web_config.wifi_ssid = WIFI_SSID;
      // web_config.wifi_password = WIFI_PASSWORD;

      web_server = etl::make_unique<webui::moonshine_web_server>(web_config);
      if(web_server && web_server->begin()) {
        // Установка функции получения состояния
        web_server->set_state_getter(get_moonshine_state);

        // Вывод информации о подключении
        Serial.println(F("\n=== Web Server Info ==="));
        Serial.print(F("Mode: "));
        Serial.println(web_server->get_mode());
        Serial.print(F("IP Address: "));
        Serial.println(web_server->get_ip_address());
        Serial.print(F("Hostname: http://"));
        Serial.print(web_server->get_mode() == "AP" ? web_server->get_ip_address() : "moonshine.local");
        Serial.println(F("/"));
        Serial.println(F("=========================\n")); 
      }      
      else {
        Serial.println(F("[ERROR] Web server initialization failed!"));
      }    
    }
  }

void check_temperature(etl::shared_ptr<temperature_sensor_t> t)
{
  if (t->tick()) 
  {
      String uptime = settings::moonshine::get_uptime_string();
      // Serial.print(uptime);
      // Serial.print(" ");
      // Serial.print(t->name);
      // Serial.print(": ");
      // if(auto value = t->temperature(); value)
      // {
      //   Serial.print(*value, 2);
      //   Serial.print("°C, ");
      // }
      // else{
      //   Serial.print("---    ");
      // }   
      // Serial.print(t->title);
      // Serial.println();

      // Uptime
      static String last_uptime = "";// оптимизация вывода времени
      if(last_uptime != uptime)
      {
        lcd.setCursor(12, 3);
        lcd.print(uptime);
        last_uptime = uptime;
      }

      int y = 0;
      for(auto s : t_sensors)
      {
        if(s->name == t->name) break;
        y++;
      }
      lcd.setCursor(0, y);
      lcd.print(t->name); lcd.print(":");

      etl::optional<float> value = t->temperature(); // current temperature value
      if(value)
      {
        lcd.print(*value, 1);
        lcd.print("c  ");
      }
      else{
        lcd.print("---    ");
      }   

      // update state (я знаю, что некраисво, но пусть уж так)
      settings::moonshine::state_t state = get_moonshine_state();
      
      if(t_top_column && t_top_column->name == t->name) {
        state.steam_temperature = value;
      }
      else if(t_heater_tank && t_heater_tank->name == t->name) {
        state.heater_temperature = value;
      }
      else if(t_deflegmator_water_out && t_deflegmator_water_out->name == t->name) {
        state.deflegmater_temperature = value;
      }
      else if(t_condenser_water_out && t_condenser_water_out->name == t->name) {
        state.condenser_temperature = value;
      }

      update_moonshine_state(state);
  }
}

void check_flow(etl::shared_ptr<flow_sensor_t> f)
{
  if (f && f->tick()) 
  {
    int y = 1;
    for(auto s : f_sensors)
    {
      if(s->name == f->name) break;
      y++;
    }
    lcd.setCursor(9, y);
    lcd.print(f->get_spinner_state());
    if(f->is_calibrate())
    {
      if(auto r = f->get_calibrate(); r) {
        lcd.setCursor(10, y);
        lcd.print(f->format_calibrate_data(r));
      }
    }
  }
}

void loop() 
{
  for(auto t : t_sensors){
    check_temperature(t);
  }

  for(auto f : f_sensors){
    check_flow(f);
  }

  if(btn && btn->tick())
  {
    //Serial.println("Button tick");
    if(btn->click())
    {
      // Напечатать количество нажатий на кнопки
      Serial.println("Button click");
      int count = ++btn_clicks;
      lcd.setCursor(15,0);
      lcd.print(count);

      for(auto f : f_sensors){
        f->set_calibrate(!f->is_calibrate());
      }
    }

    // проверка на количество кликов
    if (btn->hasClicks(3)) {
      Serial.println("has 3 clicks - start wi-fi setup");
    }
  }

  // Обработка клиентских запросов
  if(web_server)
  {
    web_server->handle_client();
  }
}