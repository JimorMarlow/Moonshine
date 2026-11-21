#include <Arduino.h>

// Вся конфигурация в pinout.h
#include "pinout.h"
#include "settings.h"
#include "version.h"
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
etl::shared_ptr<flow_sensor_t> flow_deflegmator = etl::make_shared<flow_sensor_SEN_HZ06D>("D", "flow_deflegmator", F_DEFLEGMATOR_WATER_PIN); 
// Поток воды на выходе из охладителя (конденсатора/холодильника). Нужен для контроля температуры по расходу воды
etl::shared_ptr<flow_sensor_t> flow_condenser = etl::make_shared<flow_sensor_SEN_HZ06D>("C", "flow_condenser", F_CONDENSER_WATER_PIN); 
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
//#include <GyverLCD.h>
//GyverLCD lcd(0x27, 20, 4);  // Address, columns, rows [web:8]
#include <GyverLCD_UTF8.h>
GyverLCD_UTF8 lcd(0x27, 20, 4);
// Для печати символа градуса можно сделать так:
// lcd.print("Температура: 23°C");  // Рекомендованный
// lcd.write(223);                  // символ градуса в стандартной таблице HD44780 имеет код 223 (0xDF), и его можно вывести через write()


#include "EncButton.h"
etl::shared_ptr<Button> btn = etl::make_shared<Button>(TOUCH_BTN_PIN);
volatile int btn_clicks = 0;  // для проверки нажатий кнопки

//////////////////////////////////////////////////////////////
// WEB-UI
#include "web-ui.h"
// Менеджер веб-серверов: в обычном режиме работает web-ui,
// сервер настроек (сеть, тема, шрифт) запускается только по запросу
etl::unique_ptr<webui::moonshine_web_manager> web_manager;
bool use_web_ui = true;

bool start_web_ui(bool start_settings) { // Moonshine web-ui
  if(!use_web_ui)
  {
    return false;
  }

  // Настройки WiFi и интерфейса в энергонезависимой памяти (ETL)
  Serial.print("[Settings] wifi init: ... ");
  bool is_wifi_init = etl::webui::settings::init_wifi_config(webui::make_default_config());
  Serial.println(is_wifi_init ? "OK" : "FAILED");
  etl::webui::settings::init_ui_config(webui::make_default_ui_config());

  // Функция получения состояния системы для сервера контента
  webui::moonshine_web_server::set_state_getter(get_moonshine_state);

  web_manager = etl::make_unique<webui::moonshine_web_manager>(webui::make_device_info());
  if(!web_manager)
  {
    return false;
  }

  // При зажатой кнопке при старте - сервер настроек, иначе - web-ui
  if(!web_manager->start(start_settings))
  {
    Serial.println(F("[ERROR] Web manager start failed"));
    return false;
  }

  return web_manager->trace_connection();
}

///////////////////////////////////////////
void setup() {
    Serial.begin(115200);
    if(SERIAL_INIT_DELAY > 0) delay(SERIAL_INIT_DELAY);  // задержка, чтобы отладочный вывод успевал появиться
    
    // Display
    lcd.begin();
    lcd.setBacklight(true);
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
      attachInterrupt(digitalPinToInterrupt(flow_deflegmator->get_pin()), pulse_counter_D, FALLING);
    }

    if(flow_condenser && flow_condenser->init(flow_condenser)) {
      attachInterrupt(digitalPinToInterrupt(flow_condenser->get_pin()), pulse_counter_C, FALLING);
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
      start_web_ui(is_btn_hold_on_start);
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
      if(s->get_name() == f->get_name()) break;
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
      btn_clicks += 1;
      int count = btn_clicks;
      lcd.setCursor(15,0);
      lcd.print(count);

      for(auto f : f_sensors){
        f->set_calibrate(!f->is_calibrate());
      }
    }

    // проверка на количество кликов - переключение на сервер настроек
    if (btn->hasClicks(3)) {
      Serial.println("has 3 clicks - start wi-fi setup");
      if(web_manager) {
        if(auto server = web_manager->get_server()) {
          server->schedule_settings_cb();
        }
      }
    }
  }

  // Обработка клиентских запросов и переключение серверов
  if(web_manager)
  {
    web_manager->tick();
    web_manager->handle_pending_switch();
  }

}