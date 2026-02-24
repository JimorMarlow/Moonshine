#include "settings.h"

namespace settings
{
namespace moonshine 
{

    // Функция возвращает время работы в формате HH:MM:SS
String get_uptime_string() {
  return format_uptime_string(millis());
}
  
String format_uptime_string(unsigned long uptime_ms) {

  // Вычисляем время в секундах
  unsigned long totalSeconds = uptime_ms / 1000;
  
  // Вычисляем часы, минуты, секунды
  unsigned long hours = totalSeconds / 3600;
  unsigned long minutes = (totalSeconds % 3600) / 60;
  unsigned long seconds = totalSeconds % 60;
  
  // Форматируем строку с ведущими нулями
  char buffer[16]; // с запасом, чтобы не было предупреждений
  snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu", hours, minutes, seconds);
  
  return String(buffer);
}

}// moonshine
}// settings