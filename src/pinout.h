#pragma once
#include <Arduino.h>
#include "version.h"

const int SERIAL_INIT_DELAY = 0;    // Для ESP8266 не нужно задержки
const int DS1820_TEMPEATURE_PIN = 6;  // GPIO6 (MOSI) Опрос температуры по I2C
