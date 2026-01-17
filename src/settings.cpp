#include "settings.h"

namespace settings
{
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
}// settings