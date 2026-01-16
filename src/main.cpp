#include <Arduino.h>

// Вся конфигурация и платформозависимые настройки в pinout.h
#include "pinout.h"

//////////////////////////////////////////////////////////
#include "etl/etl_memory.h"
#include "etl/etl_optional.h"
#include <GTimer.h>

void setup() {
    Serial.begin(115200);
    if(SERIAL_INIT_DELAY > 0) delay(SERIAL_INIT_DELAY);  // для ESP32 C3 supermini нуждо сделать задержку, чтобы выводилась отладочная информация
    
    Serial.printf("Moonshine v%s started...\n", String(MS_VERSION_STRING).c_str());
  
    Serial.println("waterflow control started...");
}

void loop() 
{
}