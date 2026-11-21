#pragma once
#include <Arduino.h>

// Назначение пинов платы Moonshine (ESP32 WROOM-38U)
//
// Пин на ESP32 | GPIO № | Куда идет (Компонент -> Пин) | Назначение / Примечание
// | :--- | :--- | :--- | :--- |
// | **D21** | 21 | LCD2004 (`SDA`) | Шина I2C (данные)
// | **D22** | 22 | LCD2004 (`SCL`) | Шина I2C (тактирование)
// | **D15** | 15 | Кнопка (`1.l`) | Вход кнопки. В коде обязательно включить `INPUT_PULLUP`
// | **D4** | 4 | Датчик Steam (`DQ`) | Шина 1-Wire (пар)
// | **D5** | 5 | Датчик Heater (`DQ`) | Шина 1-Wire (нагреватель)
// | **D18** | 18 | Датчик DF (`DQ`) | Шина 1-Wire (дефлегматор)
// | **D19** | 19 | Датчик Cond (`DQ`) | Шина 1-Wire (конденсатор/холодильник)
// | **D32** | 32 | Преобразователь (`LV1`) | Сигнал от датчика потока `flowDF` (после понижения уровня)
// | **D33** | 33 | Преобразователь (`LV2`) | Сигнал от датчика потока `flowCond` (после понижения уровня)

// Задержка после Serial.begin(): без неё отладочный вывод теряется при старте
const int SERIAL_INIT_DELAY = 1000;

const int T_HEATER_TANK_PIN = 5;            // - D5 t_heater_tank
const int T_DEFLEGMATOR_WATER_OUT_PIN = 18; // - D18 t_deflegmator_water_out
const int T_CONDENSER_WATER_OUT_PIN = 19;   // - D19 t_condenser_water_out
const int T_TOP_COLUMN_PIN = 4;             // - D4 t_top_column

const int F_DEFLEGMATOR_WATER_PIN = 32;     // - D32 flow_deflegmator_water
const int F_CONDENSER_WATER_PIN = 33;       // - D33 flow_condenser_water

const int TOUCH_BTN_PIN = 15;               // - D15 touch_btn
