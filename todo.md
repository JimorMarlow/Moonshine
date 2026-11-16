# Knbown problems

Найденные проблемы
1. Поток воды вообще не доходит до веб-интерфейса — самое заметное. Поля deflegmater_water_flow / condenser_water_flow есть в src/settings.h и отдаются в JSON (src/web-ui.cpp), но в src/main.cpp check_flow() только рисует спиннер на LCD и ничего не пишет в state_t. Итог: на дашборде поток всегда 0 ml/min, а правило статуса (src/web-ui.cpp) при баке >55 °C и DFT >30 °C выдаёт красные «Нет воды в охладителе/дефлегматоре!» даже когда вода идёт. Плюс сам поток в ml/min нигде не считается (в README это честно названо калибровкой импульсов).

2. Битый кусок JS после переноса макета. src/web-ui.cpp: e[t.toLowerCase().replace(".","_")] даёт ключи dfflow/coolflow, которых в JSON нет (там deflegmater_water_flow/condenser_water_flow), поэтому n === undefined: колёса крутятся всегда с максимальной скоростью, иконка предупреждения не показывается никогда, а в группу колеса добавляется класс трубы flow-wheel-*; поиск DFFlowWaterInPipe — мёртвый код (реальные id dfWaterInPipe). В макете было sensorData[sensor] (qwen-distillation.045.html).

3. Wi-Fi setup из v1.2.11 — только бэкенд. Класс etl::wifi::wifi_setup нигде не создаётся, маршрутов /api/scan|connect|save|reset|ap_settings нет (в src/web-ui.cpp только три), макет qwen-wifi_setup.008.html не подключён, а hasClicks(3) в src/main.cpp лишь печатает «start wi-fi setup». Коммит честно говорил «не проверено».

4. Риск по куче при отдаче главной страницы. src/web-ui.cpp делает String html = FPSTR(HTML_PAGE) — страница 26 140 символов копируется в кучу при каждом GET /, затем replace может потребовать ещё буфер. При 30.5 КБ статики и работающем Wi-Fi на ESP8266 это близко к пределу: в худшем случае пустая страница или незаменённый ${VERSION}. Надёжнее отдавать частями из PROGMEM (sendContent_P) или хранить страницу в LittleFS.

5. Мелочи, но стоит знать:

src/settings.h: опционалы инициализированы 0.0, поэтому до первого замера дашборд показывает 0.0°, а не --.-.
src/etl_wifi_setup.cpp: reset_settings() сбрасывает на server_config_t() → espdevice / ESP_Device_AP, а не на проектные moonshine / Moonshine_AP.
LittleFS.begin()/end() в save/load/reset размонтирует ФС; если позже положить UI в LittleFS — сломается. Кстати, AGENTS.md утверждает, что UI отдаётся из LittleFS, а на деле из PROGMEM.
src/etl_wifi_setup.h использует std::vector/std::sort без <vector>/<algorithm> — сейчас тянется транзитивно, хрупко.
src/sensor_flow.cpp: total_count += pulse_count; pulse_count = 0; может потерять импульс между чтением и обнулением (редко, только на высоких потоках).
src/pinout.h: F_DEFLEGMATOR_WATER_PIN = 10 — это GPIO10 (SD_D3, ножка флеша). Работает только при DIO/DOUT-режиме флеша; при QIO будут отказы. Пины не трогал (по AGENTS.md), просто уточни.