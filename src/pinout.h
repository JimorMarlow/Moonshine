#pragma once
#include <Arduino.h>
#include "version.h"

const int SERIAL_INIT_DELAY = 0;    // Для ESP8266 не нужно задержки

const int T_HEATER_TANK_PIN = D5; // - D5 GPIO14 t_heater_tank
const int T_DEFLEGMATOR_WATER_OUT_PIN = D6; // - D6 GPIO12 t_deflegmator_water_out
const int T_CONDENSER_WATER_OUT_PIN = D7; // - D7 GPIO13 t_condenser_water_out
const int T_TOP_COLUMN_PIN = D8; // - D8 GPIO15 t_top_column