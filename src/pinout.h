#pragma once
#include <Arduino.h>
#include "version.h"


#ifdef BOARD_NODEMCU

const int SERIAL_INIT_DELAY = 0;    // Для ESP8266 не нужно задержки

/* На ESP8266 нельзя подавать внешние сигналы датчиков на определённые пины во 
время включения или сброса, иначе нарушится загрузка прошивки или возникнут
ложные срабатывания из-за стартовых импульсов. Критические пины требуют 
строгих уровней: 
D3 GPIO0 — HIGH (3.3V), 
D4 GPIO2 — HIGH (3.3V), 
D8 GPIO15 — LOW (GND), 
D0 GPIO16 — HIGH при старте.​

для дисплея заняты входы SDL, SDA
D1	GPIO5		✓	I2C SCL
D2	GPIO4		✓	I2C SDA

Все GPIO ESP8266 (NodeMCU), кроме GPIO16 (D0), 
поддерживают внешние прерывания через attachInterrupt. 
Для датчиков Холла подойдут эти входы;
D1	5	Свободно	✓	Идеальный для YF-S201
D2	4	HIGH	✓	Хорош, встроенный pullup
D5	14	Свободно	✓	Универсальный

*/
const int T_HEATER_TANK_PIN = D5;           // - D5 GPIO14 t_heater_tank
const int T_DEFLEGMATOR_WATER_OUT_PIN = D6; // - D6 GPIO12 t_deflegmator_water_out
const int T_CONDENSER_WATER_OUT_PIN = D7;   // - D7 GPIO13 t_condenser_water_out
const int T_TOP_COLUMN_PIN = D4;            // - D4 GPIO02 t_top_column

const int F_DEFLEGMATOR_WATER_PIN = 10;     // - S3 GPIO10 flow_deflegmator_water
const int F_CONDENSER_WATER_PIN = 3;        // RX GPIO3 -  flow_condenser_water

const int TOUCH_BTN_PIN = D3;               // - D3 GPIO0 touch_btn

#elif defined(BOARD_ESP32_WROOM_38U)
 
const int SERIAL_INIT_DELAY = 1000; // для ESP32 C3нуждо сделать задержку, чтобы выводилась отладочная информация

/* 

Пин на ESP32 | GPIO № | Куда идет (Компонент -> Пин) | Назначение / Примечание |
| :--- | :--- | :--- | :--- |
| **D21** | 21 | LCD2004 (`SDA`) | Шина I2C (данные) |
| **D22** | 22 | LCD2004 (`SCL`) | Шина I2C (тактирование) |
| **D15** | 15 | Кнопка (`1.l`) | Вход кнопки. В коде обязательно включить `INPUT_PULLUP` |
| **D4** | 4 | Датчик Steam (`DQ`) | Шина 1-Wire (пар) |
| **D5** | 5 | Датчик Heater (`DQ`) | Шина 1-Wire (нагреватель) |
| **D18** | 18 | Датчик DF (`DQ`) | Шина 1-Wire (дефлегматор) |
| **D19** | 19 | Датчик Cond (`DQ`) | Шина 1-Wire (конденсатор/холодильник) |
| **D32** | 32 | Преобразователь (`LV1`) | Сигнал от датчика потока `flowDF` (после понижения уровня) |
| **D33** | 33 | Преобразователь (`LV2`) | Сигнал от датчика потока `flowCond` (после понижения уровня) |

*/
const int T_HEATER_TANK_PIN = 5;           // - D5 t_heater_tank
const int T_DEFLEGMATOR_WATER_OUT_PIN = 18; // - D18 t_deflegmator_water_out
const int T_CONDENSER_WATER_OUT_PIN = 19;   // - D19 t_condenser_water_out
const int T_TOP_COLUMN_PIN = 4;            // - D4 t_top_column

const int F_DEFLEGMATOR_WATER_PIN = 32;     // - D32 flow_deflegmator_water
const int F_CONDENSER_WATER_PIN = 33;        // - D33 flow_condenser_water

const int TOUCH_BTN_PIN = 15;               // - D15 touch_btn

#else
    #error "Pinout setup: unsupported platform (need ETL_ESP8266 or ETL_ESP32)"
#endif

