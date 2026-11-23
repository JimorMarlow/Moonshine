/**
 * @file web-ui.cpp
 * @brief Реализация веб-сервера Moonshine для ESP32
 * 
 * @note Для обновления макета:
 *       1. Взять актуальный HTML из docs/web-ui/qwen-distillation.xxx.html
 *       2. Заменить версию в заголовке на ${MS_VERSION_STRING}
 *       3. Удалить логику симуляции (simState, sensorData, updateSimulationLogic)
 *       4. Обновить JavaScript для получения данных через AJAX с /api/state
 */

#include "web-ui.h"
#include "version.h"
#include <pgmspace.h>

namespace webui
{
    // HTML шаблон веб-интерфейса
    // Для обновления: взять HTML из docs/web-ui/qwen-distillation.xxx.html
    // и заменить версию в заголовке на актуальную
    static const char HTML_PAGE_A[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang=")rawliteral";

    static const char HTML_PAGE_B[] PROGMEM = R"rawliteral(">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>Moonshine - Monitoring</title>
    <style>
        *{margin:0;padding:0;box-sizing:border-box}
        body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,'Helvetica Neue',Arial,sans-serif;background:#f5f5f5;min-height:100vh;color:#2c3e50;display:flex;justify-content:center;align-items:flex-start;padding:2px}
        .container{width:100%;max-width:1400px;margin:0 auto}
        .main-panel{display:grid;grid-template-columns:minmax(530px,1fr) 300px;gap:5px;height:calc(100vh - 4px)}
        .scheme-container{background:#fff;border-radius:8px;padding:0;box-shadow:0 2px 8px rgba(0,0,0,0.08);border:1px solid #e0e0e0;position:relative;overflow:hidden;height:100%;display:flex;flex-direction:column;min-width:0}
        .sensors-panel{background:#fff;border-radius:8px;padding:15px;box-shadow:0 2px 8px rgba(0,0,0,0.08);border:1px solid #e0e0e0;min-width:250px;height:100%;display:flex;flex-direction:column;overflow:hidden}
        .sensors-content{flex:1;overflow-y:auto}
        .distillation-svg{width:100%;height:100%;display:block}
        .heater{fill:#fff;stroke-width:2.5}.heater-liquid{fill:#3498db;opacity:0.3}.column{stroke:none}.deflegmator{fill:#fff;stroke:#27ae60;stroke-width:2.5}.condenser{fill:#fff;stroke:#3498db;stroke-width:2.5}
        .pipe{fill:none;stroke:#95a5a6;stroke-width:4;stroke-linecap:round}.vapor-pipe{stroke-width:12;stroke-linecap:round;stroke-linejoin:round}.water-pipe-in{stroke:#3498db;stroke-width:4}.water-pipe-out{stroke-width:4}.product-pipe{stroke:#95a5a6;stroke-width:5}
        .temp-rect{fill:#fff;stroke:#f39c12;stroke-width:1}.flow-value-rect{fill:#fff;stroke:#1abc9c;stroke-width:1}
        .temp-text,.flow-text{font-size:24px;font-weight:300;fill:#2c3e50;text-anchor:middle;font-family:'Courier New',monospace;dominant-baseline:middle}
        .flow-unit-text{font-size:12px;font-weight:500;fill:#7f8c8d;text-anchor:middle;font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;dominant-baseline:middle}
        .temp-label{font-size:14px;fill:#7f8c8d;text-anchor:middle;font-weight:500}
        .sensor-flow-rect{fill:#1abc9c;stroke:#16a085;stroke-width:2;cursor:pointer}
        .flow-wheel-group{transform-box:fill-box;transform-origin:center}.flow-wheel-slow{animation:rotate 3s linear infinite}.flow-wheel-medium{animation:rotate 1.5s linear infinite}.flow-wheel-fast{animation:rotate 0.7s linear infinite}
        .flow-wheel{fill:none;stroke:#fff;stroke-width:2}.flow-wheel-line{stroke:#fff;stroke-width:2;stroke-linecap:round}
        .warning-icon{display:none}.warning-icon path{fill:#f1c40f;stroke:#d35400;stroke-width:1}.warning-icon text,.warning-icon circle{fill:#2c3e50;font-weight:900;font-family:sans-serif}
        @keyframes rotate{from{transform:rotate(0deg)}to{transform:rotate(360deg)}}
        .flow-slow{stroke-dasharray:10,10;animation:flowAnimation 2s linear infinite}.flow-medium{stroke-dasharray:8,8;animation:flowAnimation 1s linear infinite}.flow-fast{stroke-dasharray:5,5;animation:flowAnimation 0.5s linear infinite}
        @keyframes flowAnimation{to{stroke-dashoffset:-20}}
        .drop-blink-1hz{animation:blink 1s infinite}.drop-blink-2hz{animation:blink 0.5s infinite}.drop-blink-4hz{animation:blink 0.25s infinite}
        @keyframes blink{0%,100%{opacity:1}50%{opacity:0.2}}
        .bubble{fill:#3498db;opacity:0.6;transform-box:fill-box;transform-origin:center bottom;animation:rise linear infinite}
        .bubble-speed-1{animation-duration:5s}.bubble-speed-2{animation-duration:4s}.bubble-speed-3{animation-duration:3s}.bubble-speed-4{animation-duration:2s}.bubble-speed-5{animation-duration:1.5s}.bubble-speed-6{animation-duration:1s}.bubble-speed-7{animation-duration:0.5s}
        @keyframes rise{0%{transform:translateY(0) scale(0.5);opacity:0}20%{opacity:0.6}100%{transform:translateY(-60px) scale(1.2);opacity:0}}
        .sensor-card{background:#fafafa;border-radius:6px;padding:10px;margin-bottom:10px;border-left:4px solid;transition:all 0.3s}.sensor-card:hover{background:#f0f0f0}
        .temp-sensor{border-left-color:#f39c12}.flow-sensor{border-left-color:#1abc9c}
        .sensor-title{font-size:12px;color:#7f8c8d;margin-bottom:6px;font-weight:500;line-height:1.2}
        .temp-display{display:flex;align-items:baseline;justify-content:flex-start}
        .temp-value{font-size:28px;font-weight:300;font-family:'Courier New',monospace;color:#2c3e50;line-height:1}
        .temp-unit{font-size:12px;color:#95a5a6;margin-left:4px}
        .flow-value{font-size:24px;font-weight:300;font-family:'Courier New',monospace;color:#2c3e50}
        .flow-unit{font-size:11px;color:#95a5a6;margin-left:4px}
        .section-title{font-size:14px;margin-bottom:12px;padding-bottom:6px;border-bottom:2px solid #ecf0f1;color:#2c3e50;font-weight:500}
        .update-time{font-size:10px;color:#95a5a6;margin-top:6px}
        .internal-header{text-align:center;padding:5px;margin-bottom:5px;border-bottom:1px solid #ecf0f1;color:#2c3e50;font-weight:600;font-size:14px;letter-spacing:1px}
        .status-bar{padding:8px;margin:5px 10px;border-radius:5px;font-size:15px;font-weight:400;text-align:center;border:1px solid;display:flex;align-items:center;justify-content:center;gap:8px}
        .status-ok{background:#d5f5e3;color:#27ae60;border-color:#27ae60}.status-warning{background:#f9e79f;color:#9a7d0a;border-color:#f1c40f}.status-error{background:#fadbd8;color:#c0392b;border-color:#e74c3c}
        .status-icon{width:16px;height:16px;display:inline-block}
        .uptime-card{display:flex;align-items:center;justify-content:space-between;background:#fafafa;border-radius:6px;padding:10px;margin-bottom:15px;border-left:4px solid #2c3e50}
        .uptime-value{font-size:28px;font-weight:300;font-family:'Courier New',monospace;color:#2c3e50;line-height:1}
        .bold-btn{width:36px;height:36px;border:1px solid #bdc3c7;border-radius:6px;background:#fff;color:#2c3e50;font-size:18px;font-weight:600;cursor:pointer;display:flex;align-items:center;justify-content:center;transition:all 0.2s;flex-shrink:0}
        .bold-btn:hover{background:#ecf0f1}
        .bold-btn.active{background:#3498db;color:#fff;border-color:#2980b9}
        .bold-values .temp-value,.bold-values .flow-value,.bold-values .temp-text,.bold-values .flow-text,.bold-values .uptime-value{font-weight:700}
        /* Тёмная тема: цвета как у сервера настроек ETL */
        body.dark-theme{background:#1C1C1E;color:#FFFFFF}
        body.dark-theme .scheme-container,body.dark-theme .sensors-panel{background:#2C2C2E;border-color:#38383A;box-shadow:none}
        body.dark-theme .sensor-card{background:#3A3A3C}
        body.dark-theme .sensor-card:hover{background:#48484A}
        body.dark-theme .sensor-title,body.dark-theme .update-time,body.dark-theme .temp-unit,body.dark-theme .flow-unit{color:#98989D}
        body.dark-theme .temp-value,body.dark-theme .flow-value,body.dark-theme .uptime-value{color:#FFFFFF}
        body.dark-theme .section-title,body.dark-theme .internal-header{color:#FFFFFF;border-bottom-color:#38383A}
        body.dark-theme .uptime-card{background:#3A3A3C;border-left-color:#0A84FF}
        body.dark-theme .bold-btn{background:#3A3A3C;border-color:#48484A;color:#98989D}
        body.dark-theme .bold-btn:hover{background:#007AFF;border-color:#007AFF;color:#FFFFFF}
        body.dark-theme .temp-rect,body.dark-theme .flow-value-rect{fill:#2C2C2E;stroke:#48484A}
        body.dark-theme .heater,body.dark-theme .deflegmator,body.dark-theme .condenser{fill:#2C2C2E}
        body.dark-theme .temp-text,body.dark-theme .flow-text{fill:#FFFFFF}
        body.dark-theme .temp-label,body.dark-theme .flow-unit-text{fill:#98989D}
        body.dark-theme .svg-label{fill:#FFFFFF}
        body.dark-theme .svg-label.muted{fill:#98989D}
        body.dark-theme .status-bar.status-ok{background:#12301D;color:#30D158;border-color:#30D158}
        body.dark-theme .status-bar.status-warning{background:#332B0A;color:#FFD60A;border-color:#FFD60A}
        body.dark-theme .status-bar.status-error{background:#3A1512;color:#FF453A;border-color:#FF453A}
        /* Увеличенный шрифт */
        body.large-font .section-title{font-size:17px}
        body.large-font .sensor-title{font-size:14px}
        body.large-font .temp-value{font-size:34px}
        body.large-font .flow-value{font-size:29px}
        body.large-font .uptime-value{font-size:34px}
        body.large-font .temp-unit,body.large-font .flow-unit{font-size:13px}
        body.large-font .temp-text{font-size:30px}
        body.large-font .flow-text{font-size:28px}
        body.large-font .flow-unit-text{font-size:14px}
        body.large-font .temp-label{font-size:16px}
        body.large-font .status-bar{font-size:17px}
        body.large-font .update-time{font-size:12px}
        @media(max-width:900px){.main-panel{grid-template-columns:1fr;grid-template-rows:auto auto;height:auto}.scheme-container{height:auto;min-height:480px;aspect-ratio:530/480}.sensors-panel{height:auto;max-height:none;overflow:visible}.sensors-content{overflow-y:visible}.bold-btn{width:40px;height:40px;font-size:20px}.uptime-value{font-size:24px}}
    </style>
</head>
<body class=")rawliteral";

    // Страница отдаётся частями прямо из PROGMEM: плейсхолдеры подставляются между кусками,
    // поэтому копия ~31 КБ в кучу на каждый запрос не нужна (экономия и меньше фрагментации).
    static const char HTML_PAGE_C[] PROGMEM = R"rawliteral(">
    <div class="container">
        <div class="main-panel">
            <div class="scheme-container">
                <div class="internal-header">Moonshine v)rawliteral";
    static const char HTML_PAGE_D[] PROGMEM = R"rawliteral(</div>
                <div id="statusBar" class="status-bar status-ok">
                    <span class="status-icon" id="statusIcon">✓</span>
                    <span id="statusText" data-i18n="status_ok">Система работает нормально</span>
                </div>
                <svg class="distillation-svg" viewBox="0 0 530 480" xmlns="http://www.w3.org/2000/svg" preserveAspectRatio="xMidYMid meet">
                    <defs>
                        <linearGradient id="columnGradient" x1="0%" y1="100%" x2="0%" y2="0%"><stop offset="0%" stop-color="#3498db" id="colStopLow"/><stop offset="100%" stop-color="#3498db" id="colStopHigh"/></linearGradient>
                        <linearGradient id="vaporGradient" x1="0%" y1="0%" x2="100%" y2="0%"><stop offset="0%" stop-color="#3498db" id="vapStopLow"/><stop offset="100%" stop-color="#3498db" id="vapStopHigh"/></linearGradient>
                        <linearGradient id="dfOutGradient" x1="0%" y1="100%" x2="0%" y2="0%"><stop offset="0%" stop-color="#3498db" id="dfOutStopLow"/><stop offset="100%" stop-color="#3498db" id="dfOutStopHigh"/></linearGradient>
                        <linearGradient id="coolOutGradient" x1="0%" y1="100%" x2="0%" y2="0%"><stop offset="0%" stop-color="#3498db" id="coolOutStopLow"/><stop offset="100%" stop-color="#3498db" id="coolOutStopHigh"/></linearGradient>
                        <g id="warningSymbol"><path d="M 0,-11 L 9,6 L -9,6 Z" fill="#f1c40f" stroke="#d35400" stroke-width="1"/><path d="M 0,-5 L 0,1" stroke="#2c3e50" stroke-width="2.5" stroke-linecap="round"/><circle cx="0" cy="4" r="1.2" fill="#2c3e50"/></g>
                        <path id="dropShape" d="M 0,0 Q -4,-4 0,-10 Q 4,-4 0,0" fill="#3498db" opacity="0.8"/>
                    </defs>
                    <rect x="210" y="200" width="30" height="150" class="column" fill="url(#columnGradient)" id="column"/>
                    <rect x="165" y="350" width="120" height="80" rx="5" class="heater" id="heater"/>
                    <rect x="167" y="366" width="116" height="62" rx="3" class="heater-liquid" id="heaterLiquid"/>
                    <g id="heaterBubbles">
                        <circle cx="180" cy="420" r="2" class="bubble bubble-speed-1" style="animation-delay:0s;"/>
                        <circle cx="200" cy="415" r="2.5" class="bubble bubble-speed-1" style="animation-delay:1.2s;"/>
                        <circle cx="220" cy="425" r="2" class="bubble bubble-speed-1" style="animation-delay:2.4s;"/>
                        <circle cx="240" cy="410" r="2.5" class="bubble bubble-speed-1" style="animation-delay:0.6s;"/>
                        <circle cx="260" cy="420" r="2" class="bubble bubble-speed-1" style="animation-delay:3.0s;"/>
                        <circle cx="190" cy="405" r="1.8" class="bubble bubble-speed-1" style="animation-delay:1.8s;"/>
                        <circle cx="210" cy="400" r="2.2" class="bubble bubble-speed-1" style="animation-delay:3.6s;"/>
                        <circle cx="230" cy="408" r="1.8" class="bubble bubble-speed-1" style="animation-delay:0.9s;"/>
                    </g>
                    <text x="225" y="402" text-anchor="middle" fill="#2c3e50" font-size="14" font-weight="500" class="svg-label">Heater</text>
                    <rect x="185" y="354" width="85" height="32" rx="3" class="temp-rect" id="rectHeater"/>
                    <text x="227.5" y="370" class="temp-text" id="valueHeaterNum">--.-°</text>
                    <rect x="195" y="405" width="60" height="12" rx="3" fill="#e74c3c" opacity="0.3"/>
                    <rect x="185" y="37" width="85" height="32" rx="3" class="temp-rect" id="rectSteam"/>
                    <text x="227.5" y="53" class="temp-text" id="valueSteamNum">--.-°</text>
                    <text x="225" y="32" text-anchor="middle" class="temp-label">Steam</text>
                    <path d="M 225 110 L 225 80 L 315 80 L 315 110" class="vapor-pipe" stroke="url(#vaporGradient)" fill="none"/>
                    <path d="M 315 200 L 315 240" class="product-pipe" id="productPipe"/>
                    <text x="315" y="255" text-anchor="middle" fill="#7f8c8d" font-size="10" class="svg-label muted">Product</text>
                    <g id="dropIndicator" transform="translate(315,270)" style="display:none"><use href="#dropShape"/></g>
                    <rect x="185" y="110" width="80" height="90" rx="5" class="deflegmator" id="deflegmator"/>
                    <text x="225" y="155" text-anchor="middle" fill="#2c3e50" font-size="10" class="svg-label">Deflegmator</text>
                    <rect x="210" y="120" width="30" height="70" rx="3" fill="#27ae60" opacity="0.2"/>
                    <rect x="275" y="110" width="80" height="90" rx="5" class="condenser" id="condenser"/>
                    <text x="315" y="155" text-anchor="middle" fill="#2c3e50" font-size="10" class="svg-label">Condenser</text>
                    <rect x="300" y="120" width="30" height="70" rx="3" fill="#3498db" opacity="0.2"/>
                    <path d="M 175 190 L 225 190 L 225 165" class="pipe water-pipe-in" id="dfWaterInPipe"/>
                    <path d="M 225 135 L 132.5 135 L 132.5 117" class="pipe water-pipe-out" id="dfWaterOutPipe"/>
                    <path d="M 365 190 L 315 190 L 315 165" class="pipe water-pipe-in" id="coolWaterInPipe"/>
                    <path d="M 315 135 L 407.5 135 L 407.5 117" class="pipe water-pipe-out" id="coolWaterOutPipe"/>
                    <g id="sensorDFFlow">
                        <rect x="145" y="180" width="30" height="20" rx="3" class="sensor-flow-rect"/>
                        <g class="flow-wheel-group" id="wheelDFGroup"><circle cx="160" cy="190" r="6" class="flow-wheel"/><line x1="160" y1="184" x2="160" y2="196" class="flow-wheel-line"/><line x1="154" y1="190" x2="166" y2="190" class="flow-wheel-line"/></g>
                        <g id="warningDF" class="warning-icon" transform="translate(160,192)"><use href="#warningSymbol"/></g>
                        <rect x="120" y="207" width="80" height="38" rx="3" class="flow-value-rect"/>
                        <text x="160" y="222" class="flow-text" id="svgValueDFFlow">0</text>
                        <text x="160" y="238" class="flow-unit-text">ml/min</text>
                    </g>
                    <g id="sensorCoolFlow">
                        <rect x="365" y="180" width="30" height="20" rx="3" class="sensor-flow-rect"/>
                        <g class="flow-wheel-group" id="wheelCoolGroup"><circle cx="380" cy="190" r="6" class="flow-wheel"/><line x1="380" y1="184" x2="380" y2="196" class="flow-wheel-line"/><line x1="374" y1="190" x2="386" y2="190" class="flow-wheel-line"/></g>
                        <g id="warningCool" class="warning-icon" transform="translate(380,192)"><use href="#warningSymbol"/></g>
                        <rect x="340" y="207" width="80" height="38" rx="3" class="flow-value-rect"/>
                        <text x="380" y="222" class="flow-text" id="svgValueCoolFlow">0</text>
                        <text x="380" y="238" class="flow-unit-text">ml/min</text>
                    </g>
                    <rect x="90" y="85" width="85" height="32" rx="3" class="temp-rect" id="rectDFT"/>
                    <text x="132.5" y="101" class="temp-text" id="valueDFTNum">--.-°</text>
                    <rect x="365" y="85" width="85" height="32" rx="3" class="temp-rect" id="rectCoolT"/>
                    <text x="407.5" y="101" class="temp-text" id="valueCoolTNum">--.-°</text>
                </svg>
            </div>
            <div class="sensors-panel">
                <div class="sensors-content">
                    <div class="uptime-card">
                        <div class="uptime-value" id="cardValueUptime">00:00:00</div>
                        <button class="bold-btn" id="settingsBtn" title="Настройки сети и шрифта">⚙</button>
                    </div>
                    <div class="section-title" data-i18n="section_temperature">Температура</div>
                    <div class="sensor-card temp-sensor"><div class="sensor-title" data-i18n="card_steam">Steam - Верх колонны</div><div class="temp-display"><div class="temp-value" id="cardValueSteam">--.-</div><div class="temp-unit">°C</div></div></div>
                    <div class="sensor-card temp-sensor"><div class="sensor-title" data-i18n="card_dft">DF.T - Дефлегматор (выход)</div><div class="temp-display"><div class="temp-value" id="cardValueDFT">--.-</div><div class="temp-unit">°C</div></div></div>
                    <div class="sensor-card temp-sensor"><div class="sensor-title" data-i18n="card_coolt">Cool.T - Конденсер (выход)</div><div class="temp-display"><div class="temp-value" id="cardValueCoolT">--.-</div><div class="temp-unit">°C</div></div></div>
                    <div class="sensor-card temp-sensor"><div class="sensor-title" data-i18n="card_heater">Heater - Температура в баке</div><div class="temp-display"><div class="temp-value" id="cardValueHeater">--.-</div><div class="temp-unit">°C</div></div></div>
                    <div class="section-title" style="margin-top:20px" data-i18n="section_flow">Поток воды</div>
                    <div class="sensor-card flow-sensor"><div class="sensor-title" data-i18n="card_dfflow">DF.Flow - Дефлегматор</div><div style="display:flex;align-items:baseline"><div class="flow-value" id="cardValueDFFlow">0</div><div class="flow-unit">ml/min</div></div></div>
                    <div class="sensor-card flow-sensor"><div class="sensor-title" data-i18n="card_coolflow">Cool.Flow - Конденсер</div><div style="display:flex;align-items:baseline"><div class="flow-value" id="cardValueCoolFlow">0</div><div class="flow-unit">ml/min</div></div></div>
                    <div class="update-time"><span data-i18n="updated">Обновлено:</span> <span id="lastUpdate">--:--:--</span></div>
                </div>
            </div>
        </div>
    </div>
    <script>
        // Язык подставляет сервер из ui_config
        const currentLang = ')rawliteral";
    static const char HTML_PAGE_E[] PROGMEM = R"rawliteral(';
        const TR = {
            ru: {
                page_title: 'Moonshine — мониторинг',
                settings_title: 'Настройки сети и шрифта',
                status_ok: 'Система работает нормально',
                status_no_response: 'Устройство не отвечает',
                section_temperature: 'Температура',
                section_flow: 'Поток воды',
                updated: 'Обновлено:',
                card_steam: 'Steam - Верх колонны',
                card_dft: 'DF.T - Дефлегматор (выход)',
                card_coolt: 'Cool.T - Конденсер (выход)',
                card_heater: 'Heater - Температура в баке',
                card_dfflow: 'DF.Flow - Дефлегматор',
                card_coolflow: 'Cool.Flow - Конденсер',
                warn_turn_on_cooling: 'Включите подачу воды в охладитель',
                err_no_water_condenser: 'Нет воды в охладителе!',
                warn_add_water_condenser: 'Добавьте воды в охладитель',
                err_no_water_deflegmator: 'Нет воды в дефлегматоре!',
                warn_add_water_deflegmator: 'Добавьте воды в дефлегматор'
            },
            en: {
                page_title: 'Moonshine — monitoring',
                settings_title: 'Network and font settings',
                status_ok: 'System is running normally',
                status_no_response: 'Device is not responding',
                section_temperature: 'Temperature',
                section_flow: 'Water flow',
                updated: 'Updated:',
                card_steam: 'Steam - Column top',
                card_dft: 'DF.T - Deflegmator (outlet)',
                card_coolt: 'Cool.T - Condenser (outlet)',
                card_heater: 'Heater - Tank temperature',
                card_dfflow: 'DF.Flow - Deflegmator',
                card_coolflow: 'Cool.Flow - Condenser',
                warn_turn_on_cooling: 'Turn on the water supply to the condenser',
                err_no_water_condenser: 'No water in the condenser!',
                warn_add_water_condenser: 'Add water to the condenser',
                err_no_water_deflegmator: 'No water in the deflegmator!',
                warn_add_water_deflegmator: 'Add water to the deflegmator'
            }
        };
        function tr(key){ return (TR[currentLang] && TR[currentLang][key]) || key; }
        function applyTranslations(){
            document.title = tr('page_title');
            const nodes = document.querySelectorAll('[data-i18n]');
            for (let i = 0; i < nodes.length; i++) nodes[i].textContent = tr(nodes[i].getAttribute('data-i18n'));
            const btn = document.getElementById('settingsBtn');
            if (btn) btn.title = tr('settings_title');
        }
        applyTranslations();
        const COLOR_BLENDER=[{temp:15,color:"#2980b9"},{temp:20,color:"#3498db"},{temp:25,color:"#f1c40f"},{temp:28,color:"#e67e22"},{temp:30,color:"#e74c3c"},{temp:35,color:"#c0392b"}];
        // Порог "нет воды", мл/мин: игольчатый кран на подаче закрывается полностью, остаточного потока нет
        const NO_WATER_ML=5;
        document.getElementById("settingsBtn").addEventListener("click",function(){location="/api/settings"});
        let lastUpdateTime=Date.now(),connectionState="connected";
        function updateConnectionState(){const e=Date.now(),t=(e-lastUpdateTime)/1e3,n=document.getElementById("statusBar"),o=document.getElementById("statusIcon"),r=document.getElementById("statusText");t>=20?(connectionState="error",n.className="status-bar status-error",o.textContent="✕",r.textContent=tr('status_no_response')):t>=10?(connectionState="warning",n.className="status-bar status-warning",o.textContent="⚠",r.textContent=tr('status_no_response')):connectionState="connected"}
        setInterval(updateConnectionState,500);
        function hexToRgb(e){const t=/^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(e);return t?{r:parseInt(t[1],16),g:parseInt(t[2],16),b:parseInt(t[3],16)}:{r:0,g:0,b:0}}
        function rgbToString(e){return`rgb(${Math.round(e.r)},${Math.round(e.g)},${Math.round(e.b)})`}
        function getTempColor(e){if(COLOR_BLENDER.length===0)return"#2c3e50";if(COLOR_BLENDER.length===1)return COLOR_BLENDER[0].color;const t=[...COLOR_BLENDER].sort(((e,t)=>e.temp-t.temp));if(e<=t[0].temp)return t[0].color;if(e>=t[t.length-1].temp)return t[t.length-1].color;for(let n=0;n<t.length-1;n++)if(e>=t[n].temp&&e<=t[n+1].temp){const o=t[n],r=t[n+1],s=(e-o.temp)/(r.temp-o.temp),a=hexToRgb(o.color),i=hexToRgb(r.color);return rgbToString({r:a.r+s*(i.r-a.r),g:a.g+s*(i.g-a.g),b:a.b+s*(i.b-a.b)})}return t[t.length-1].color}
        function formatUptime(e){const t=Math.floor(e/1e3),n=Math.floor(t/3600),o=Math.floor((t%3600)/60),r=t%60;return String(n).padStart(2,"0")+":"+String(o).padStart(2,"0")+":"+String(r).padStart(2,"0")}
        function formatTemp(e){return e.toFixed(1)}
        function getFlowWheelClass(e){return e<150?"flow-wheel-slow":e<250?"flow-wheel-medium":"flow-wheel-fast"}
        function getFlowPipeClass(e){return e<150?"flow-slow":e<250?"flow-medium":"flow-fast"}
        function updateDropIndicator(e){const t=document.getElementById("dropIndicator");if(!t)return;if(e<65)t.style.display="none",t.classList.remove("drop-blink-1hz","drop-blink-2hz","drop-blink-4hz");else{if(t.style.display="block",t.classList.remove("drop-blink-1hz","drop-blink-2hz","drop-blink-4hz"),e>=75)t.classList.add("drop-blink-4hz");else if(e>=70)t.classList.add("drop-blink-2hz");else t.classList.add("drop-blink-1hz")}}
        function updateHeaterBubbles(e){const t=document.querySelectorAll(".bubble");if(!t.length)return;let n="";e>90?n="bubble-speed-7":e>85?n="bubble-speed-6":e>80?n="bubble-speed-5":e>75?n="bubble-speed-4":e>70?n="bubble-speed-3":e>65?n="bubble-speed-2":e>60&&(n="bubble-speed-1"),t.forEach((e=>{e.classList.remove("bubble-speed-1","bubble-speed-2","bubble-speed-3","bubble-speed-4","bubble-speed-5","bubble-speed-6","bubble-speed-7"),n?(e.classList.add(n),e.style.display="block"):e.style.display="none"}))}
        function getColumnColors(e,t){return{low:getTempColor(e>35?35:e),high:getTempColor(t>35?35:t)}}
        function checkRules(e){const t=[],{Heater:n,CoolFlow:o,CoolT:r,DFT:s,DFFlow:a}=e;n.value>55&&o.value<NO_WATER_ML&&t.push({type:"warning",text:tr('warn_turn_on_cooling')}),r.value>30&&(o.value<NO_WATER_ML?t.push({type:"error",text:tr('err_no_water_condenser')}):t.push({type:"warning",text:tr('warn_add_water_condenser')})),s.value>30&&(a.value<NO_WATER_ML?t.push({type:"error",text:tr('err_no_water_deflegmator')}):t.push({type:"warning",text:tr('warn_add_water_deflegmator')}));return 0===t.length&&t.push({type:"ok",text:tr('status_ok')}),t}
        function updateStatusBar(e){if(connectionState!=="connected")return;const t=checkRules(e),n=document.getElementById("statusBar"),o=document.getElementById("statusIcon"),r=document.getElementById("statusText"),s={error:3,warning:2,ok:1},a=t.reduce(((e,t)=>s[t.type]>s[e.type]?t:e));n.className="status-bar",n.classList.add(`status-${a.type}`),o.textContent="ok"===a.type?"✓":"warning"===a.type?"⚠":"✕",r.textContent=a.text}
        function updateSensors(e){
            if(connectionState!=="connected")return;
            const dfFlow=Number(e.deflegmater_water_flow)||0,coolFlow=Number(e.condenser_water_flow)||0;
            document.getElementById("valueHeaterNum").textContent=e.heater_temperature!=null?formatTemp(e.heater_temperature)+"°":"--.-°";
            document.getElementById("valueSteamNum").textContent=e.steam_temperature!=null?formatTemp(e.steam_temperature)+"°":"--.-°";
            document.getElementById("valueDFTNum").textContent=e.deflegmater_temperature!=null?formatTemp(e.deflegmater_temperature)+"°":"--.-°";
            document.getElementById("valueCoolTNum").textContent=e.condenser_temperature!=null?formatTemp(e.condenser_temperature)+"°":"--.-°";
            document.getElementById("cardValueHeater").textContent=e.heater_temperature!=null?formatTemp(e.heater_temperature):"--.-";
            document.getElementById("cardValueSteam").textContent=e.steam_temperature!=null?formatTemp(e.steam_temperature):"--.-";
            document.getElementById("cardValueDFT").textContent=e.deflegmater_temperature!=null?formatTemp(e.deflegmater_temperature):"--.-";
            document.getElementById("cardValueCoolT").textContent=e.condenser_temperature!=null?formatTemp(e.condenser_temperature):"--.-";
            document.getElementById("cardValueDFFlow").textContent=dfFlow.toFixed(0);
            document.getElementById("cardValueCoolFlow").textContent=coolFlow.toFixed(0);
            document.getElementById("svgValueDFFlow").textContent=dfFlow.toFixed(0);
            document.getElementById("svgValueCoolFlow").textContent=coolFlow.toFixed(0);
            document.getElementById("cardValueUptime").textContent=formatUptime(e.uptime_ms);
            [{id:"DFFlow",value:dfFlow,pipe:"dfWaterInPipe"},{id:"CoolFlow",value:coolFlow,pipe:"coolWaterInPipe"}].forEach((c=>{const n=c.value,o=document.getElementById("wheel"+c.id+"Group"),r=document.getElementById("warning"+c.id),s=document.getElementById(c.pipe);if(n<NO_WATER_ML){o&&(o.style.display="none");r&&(r.style.display="block");s&&(s.classList.remove("flow-slow","flow-medium","flow-fast"),s.style.strokeDasharray="none")}else{o&&(o.style.display="block",o.classList.remove("flow-wheel-slow","flow-wheel-medium","flow-wheel-fast"),o.classList.add(getFlowWheelClass(n)));r&&(r.style.display="none");s&&(s.classList.remove("flow-slow","flow-medium","flow-fast"),s.classList.add(getFlowPipeClass(n)),s.style.strokeDasharray="")}}));
            const t=document.getElementById("dfWaterOutPipe");t&&(t.style.stroke=getTempColor(e.deflegmater_temperature||20),dfFlow>=NO_WATER_ML?(t.classList.remove("flow-slow","flow-medium","flow-fast"),t.classList.add(getFlowPipeClass(dfFlow)),t.style.strokeDasharray=""):(t.classList.remove("flow-slow","flow-medium","flow-fast"),t.style.strokeDasharray="none"));
            const n=document.getElementById("coolWaterOutPipe");n&&(n.style.stroke=getTempColor(e.condenser_temperature||20),coolFlow>=NO_WATER_ML?(n.classList.remove("flow-slow","flow-medium","flow-fast"),n.classList.add(getFlowPipeClass(coolFlow)),n.style.strokeDasharray=""):(n.classList.remove("flow-slow","flow-medium","flow-fast"),n.style.strokeDasharray="none"));
            const o=getColumnColors(e.heater_temperature||20,e.steam_temperature||20);document.getElementById("colStopLow").setAttribute("stop-color",o.low),document.getElementById("colStopHigh").setAttribute("stop-color",o.high);
            const r=getTempColor((e.steam_temperature||20)>35?35:(e.steam_temperature||20)),s=getTempColor(e.condenser_temperature||20);document.getElementById("vapStopLow").setAttribute("stop-color",r),document.getElementById("vapStopHigh").setAttribute("stop-color",s);
            const a=document.getElementById("heater");a&&(a.style.stroke=getTempColor(e.heater_temperature||20));
            updateDropIndicator(e.steam_temperature||20);
            updateHeaterBubbles(e.heater_temperature||20);
            updateStatusBar({Heater:{value:e.heater_temperature||20},CoolFlow:{value:coolFlow},CoolT:{value:e.condenser_temperature||20},DFT:{value:e.deflegmater_temperature||20},DFFlow:{value:dfFlow}});
            document.getElementById("lastUpdate").textContent=new Date().toLocaleTimeString(currentLang==="ru"?"ru-RU":"en-GB");
        }
        function fetchState(){fetch("/api/state").then((e=>e.json())).then((e=>{lastUpdateTime=Date.now();updateSensors(e)})).catch((e=>{console.error("Error fetching state:",e)}));}
        fetchState();
        setInterval(fetchState,500);
    </script>
</body>
</html>
)rawliteral";

    moonshine_web_server::moonshine_web_server()
        : etl::webui::web_server_base_t(make_default_config())
    {
    }

    void moonshine_web_server::start_http_server()
    {
        Serial.println(F("[WebUI] Starting HTTP server..."));

        const uint16_t port = get_port();
        m_server = etl::make_shared<etl_web_server_t>(port);

        m_server->on("/", HTTP_GET, [this]() { handle_root(); });
        m_server->on("/api/state", HTTP_GET, [this]() { handle_api_state(); });
        m_server->on("/api/status", HTTP_GET, [this]() { handle_api_status(); });
        m_server->on("/api/settings", HTTP_GET, [this]() { handle_api_settings(); });
        m_server->onNotFound([this]() {
            Serial.print(F("[WebUI] 404 Not Found: "));
            Serial.println(m_server->uri());
            m_server->send(404, "text/plain", "Not Found");
        });

        m_server->begin();

        Serial.print(F("[WebUI] HTTP server started on port "));
        Serial.println(port);

        // mDNS: сервис http регистрирует WiFi-менеджер при старте,
        // здесь только проверяем, что имя хоста слушается
        init_mdns(get_hostname());
    }

    moonshine_web_server::state_getter_t moonshine_web_server::s_state_getter = nullptr;

    void moonshine_web_server::set_state_getter(state_getter_t getter)
    {
        s_state_getter = getter;
    }

    void moonshine_web_server::handle_api_settings()
    {
        Serial.println(F("[WebUI] API: /api/settings"));

        // Wi-Fi при переключении не трогается, но HTTP-сервер пересоздаётся.
        // Ждём именно сервер настроек: /api/config есть только у него, поэтому старый
        // сервер контента не может «поймать» опрос раньше времени.
        m_server->sendHeader("Cache-Control", "no-cache");
        m_server->send(200, "text/html",
            F("<html><head><meta charset=\"utf-8\"><title>Moonshine</title></head>"
              "<body style=\"font-family:sans-serif;text-align:center;padding-top:40px\">"
              "<p>Переключение на сервер настроек...</p>"
              "<p style=\"color:#7f8c8d;font-size:14px\">"
              "Если страница не открылась, обновите её через несколько секунд.</p>"
              "<script>"
              "setTimeout(function(){setInterval(function(){"
              "fetch('/api/config',{cache:'no-store'}).then(function(r){"
              "if(r.ok){location.href='/';}}).catch(function(){});"
              "},1000);},1000);"
              "</script></body></html>"));

        // Переключение - после отправки ответа клиенту
        schedule_settings_cb();
    }

    // Отправка куска страницы из PROGMEM: длина куска известна на этапе компиляции
    template <size_t N>
    static void send_page_part(etl_web_server_t& server, const char (&part)[N])
    {
        server.sendContent_P(part, N - 1);
    }

    void moonshine_web_server::handle_root()
    {
        Serial.println(F("[WebUI] Serving HTML page"));

        // Настройки интерфейса: язык, тема, крупный шрифт, bold для значений
        String lang = "ru";
        String ui_class;
        if (etl::optional<etl::webui::ui_config_t> ui = get_ui_config(); ui.has_value())
        {
            if (ui->get_language() == "en") {
                lang = "en";
            }
            if (ui->is_dark_theme()) {
                ui_class += F("dark-theme ");
            }
            if (ui->is_large_font()) {
                ui_class += F("large-font ");
            }
            if (ui->is_use_bold_values()) {
                ui_class += F("bold-values ");
            }
        }

        // Страница целиком остаётся в PROGMEM и уходит частями: копии ~31 КБ в куче
        // на каждый запрос нет. Длина заранее неизвестна, поэтому HTTP/1.1 - chunked.
        m_server->setContentLength(CONTENT_LENGTH_UNKNOWN);
        m_server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        m_server->send(200, "text/html", "");

        send_page_part(*m_server, HTML_PAGE_A);
        m_server->sendContent(lang);
        send_page_part(*m_server, HTML_PAGE_B);
        m_server->sendContent(ui_class);
        send_page_part(*m_server, HTML_PAGE_C);
        m_server->sendContent_P(MS_VERSION_STRING, sizeof(MS_VERSION_STRING) - 1);
        send_page_part(*m_server, HTML_PAGE_D);
        m_server->sendContent(lang);
        send_page_part(*m_server, HTML_PAGE_E);
        m_server->sendContent("", 0);   // завершающий chunk
    }

    void moonshine_web_server::handle_api_state()
    {
        Serial.println(F("[WebUI] API: /api/state"));

        if (s_state_getter) {
            settings::moonshine::state_t state = s_state_getter();

            // JSON собирается в стековый буфер: без десятков временных String каждые 500 мс
            char json[320];
            state_to_json(state, json, sizeof(json));

            m_server->sendHeader("Cache-Control", "no-cache");
            m_server->send(200, "application/json", json);
        } else {
            m_server->send(500, "application/json", F("{\"error\":\"State getter not set\"}"));
        }
    }

    void moonshine_web_server::handle_api_status()
    {
        String json = String(F("{\"uptime_ms\":")) + millis() +
                      String(F(",\"mode\":\"")) + get_mode() +
                      String(F("\",\"ip\":\"")) + get_ip_address() +
                      String(F("\",\"hostname\":\"")) + get_hostname() +
                      String(F("\"}"));

        m_server->send(200, "application/json", json);
    }

    // Температура в JSON: число с одним знаком или null, если датчик ещё не дал показаний
    static void format_json_temperature(char* out, size_t size, const etl::optional<float>& value)
    {
        if (value) {
            snprintf(out, size, "%.1f", (double)*value);
        } else {
            snprintf(out, size, "null");
        }
    }

    size_t moonshine_web_server::state_to_json(const settings::moonshine::state_t& state, char* out, size_t size)
    {
        char steam[12], heater[12], deflegmater[12], condenser[12];
        format_json_temperature(steam, sizeof(steam), state.steam_temperature);
        format_json_temperature(heater, sizeof(heater), state.heater_temperature);
        format_json_temperature(deflegmater, sizeof(deflegmater), state.deflegmater_temperature);
        format_json_temperature(condenser, sizeof(condenser), state.condenser_temperature);

        const int length = snprintf(out, size,
            "{\"uptime_ms\":%lu,"
            "\"steam_temperature\":%s,\"heater_temperature\":%s,"
            "\"deflegmater_temperature\":%s,\"condenser_temperature\":%s,"
            "\"deflegmater_water_flow\":%.1f,\"condenser_water_flow\":%.1f}",
            (unsigned long)state.uptime_ms, steam, heater, deflegmater, condenser,
            (double)state.deflegmater_water_flow, (double)state.condenser_water_flow);

        if (length < 0) {
            if (size > 0) out[0] = '\0';
            return 0;
        }
        return (size_t)length < size ? (size_t)length : size - 1;
    }

    etl::webui::server_config_t make_default_config()
    {
        etl::webui::server_config_t cfg;
        cfg.set_hostname("moonshine");             // Имя хоста для mDNS
        cfg.set_ap_ssid("Moonshine_AP");           // SSID точки доступа
        cfg.set_ap_password("moonshine123");       // Пароль точки доступа
        cfg.set_wifi_ssid("");                     // SSID внешней Wi-Fi сети (пусто = только AP)
        cfg.set_wifi_password("moonshine123");     // Пароль внешней Wi-Fi сети
        cfg.port = 80;                             // Порт веб-сервера
        cfg.update_interval = 500;                 // Интервал обновления данных (мс)
        return cfg;
    }

    etl::webui::ui_config_t make_default_ui_config()
    {
        etl::webui::ui_config_t cfg;
        cfg.set_language("ru");                    // Язык интерфейса
        cfg.set_dark_theme(false);                 // Тёмная тема
        cfg.set_large_font(false);                 // Увеличенный шрифт
        cfg.set_use_bold_values(false);            // Bold шрифт для ключевых значений
        return cfg;
    }

    // Временная иконка устройства для страницы настроек.
    // Источник - docs/images/icon_moonshine_mini.svg: пробный упрощённый вариант,
    // его планируется заменить. При замене обновить и файл, и эту строку.
    static const char DEVICE_ICON_SVG[] PROGMEM = R"svg(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 48 52" width="127" height="138">
  <!-- Moonshine: колба с чипом, холодильник, Wi-Fi -->
  <g fill="none" stroke="#1D6FC0" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">
    <!-- колба с горловиной -->
    <path d="M12.5 25V17.5a1.5 1.5 0 0 1 1.5-1.5h4A1.5 1.5 0 0 1 19.5 17.5V25A11 11 0 1 1 12.5 25Z" fill="#FFFFFF"/>
    <!-- жидкость -->
    <path d="M6.3 33h19.4v3a9.7 9.7 0 0 1-19.4 0Z" fill="#B7E0FA" stroke="none"/>
    <!-- чип в жидкости -->
    <rect x="12" y="32" width="8" height="8" rx="1.2" fill="#FFFFFF" stroke="#1D3F6E" stroke-width="1.2"/>
    <g stroke="#1D3F6E" stroke-width="1.2">
      <path d="M9.6 34H12M9.6 38H12M20 34h2.4M20 38h2.4"/>
    </g>
    <circle cx="16" cy="36" r="1.3" fill="#1D3F6E" stroke="none"/>
    <!-- патрубок и холодильник-пробирка -->
    <path d="M25.2 30H30"/>
    <rect x="30" y="18" width="9" height="24" rx="4.5" fill="#FFFFFF"/>
    <rect x="32" y="30" width="5" height="9" rx="2.5" fill="#B7E0FA" stroke="none"/>
    <!-- термометр -->
    <rect x="21.8" y="12" width="4.8" height="13.5" rx="2.4" fill="#FFFFFF"/>
    <path d="M24.2 16v7" stroke="#E8604C" stroke-width="1.6"/>
    <circle cx="24.2" cy="25.7" r="2.7" fill="#E8604C" stroke="none"/>
    <!-- Wi-Fi -->
    <path d="M29 11a7 7 0 0 1 14 0"/>
    <path d="M32 13a4.6 4.6 0 0 1 8 0"/>
    <circle cx="36" cy="15.2" r="1.2" fill="#1D6FC0" stroke="none"/>
  </g>
</svg>)svg";

    etl::webui::device_info_t make_device_info()
    {
        etl::webui::device_info_t info;
        info.name = "Moonshine v" + String(MS_VERSION_STRING);
        info.description = "Контроль температуры дистилляционной колонны на базе ESP32";
        info.icon_svg = FPSTR(DEVICE_ICON_SVG);
        return info;
    }

    // Менеджер один на прошивку: callback'и серверов - обычные функции без контекста
    static moonshine_web_manager* s_manager = nullptr;

    // WiFi-менеджер тоже один на всё время работы. Если каждый сервер создаёт свой,
    // при переключении деструктор старого сервера освобождает его manager, а
    // manager::~manager() вызывает stop() и гасит точку доступа нового сервера.
    static etl::shared_ptr<etl::wifi::manager> s_wifi_manager;

    static etl::shared_ptr<etl::wifi::manager> get_wifi_manager()
    {
        if (!s_wifi_manager) {
            s_wifi_manager = etl::make_shared<etl::wifi::manager>(make_default_config());
        }
        return s_wifi_manager;
    }

    static void on_settings_requested()
    {
        if (s_manager) s_manager->request_settings();
    }

    static void on_content_requested()
    {
        if (s_manager) s_manager->request_content();
    }

    moonshine_web_manager::moonshine_web_manager(const etl::webui::device_info_t& device_info)
        : etl::webui::web_manager(device_info)
    {
        s_manager = this;
    }

    moonshine_web_manager::~moonshine_web_manager()
    {
        if (s_manager == this) s_manager = nullptr;
    }

    void moonshine_web_manager::request_settings()
    {
        m_settings_requested = true;
    }

    void moonshine_web_manager::request_content()
    {
        m_content_requested = true;
    }

    void moonshine_web_manager::handle_pending_switch()
    {
        if (m_settings_requested) {
            m_settings_requested = false;
            Serial.println(F("[WebManager] Switching to settings server..."));
            activate_settings();
            trace_active_server();
        } else if (m_content_requested) {
            m_content_requested = false;
            Serial.println(F("[WebManager] Switching to content server..."));
            activate_content();
            trace_active_server();
        }
    }

    void moonshine_web_manager::trace_active_server() const
    {
        if (!m_server) {
            Serial.println(F("[WebManager] No active server"));
            return;
        }

        Serial.println(F("\n=== Web Server Info ==="));
        Serial.print(F("Server:   "));
        Serial.println(m_settings_active ? F("Setup") : F("Content"));
        Serial.print(F("Mode:     "));
        Serial.println(m_server->get_mode());
        Serial.print(F("IP:       "));
        Serial.println(m_server->get_ip_address());
        Serial.print(F("Hostname: http://"));
        Serial.println(m_server->get_hostname());
        Serial.println(F("========================\n"));
    }

    void moonshine_web_server::stop_http_only()
    {
        Serial.println(F("[WebUI] stopping HTTP server"));
        // Wi-Fi не трогаем: точка доступа остаётся поднятой
        if (m_server) {
            m_server->stop();
            m_server.reset();
        }
        m_initialized = false;
        Serial.printf("[WebUI] HTTP server stopped, heap=%u\n", (unsigned)ESP.getFreeHeap());
    }

    void moonshine_web_server::start_http_only()
    {
        Serial.printf("[WebUI] starting HTTP server, heap=%u\n", (unsigned)ESP.getFreeHeap());
        start_http_server();
        m_initialized = true;
        Serial.printf("[WebUI] HTTP server started, heap=%u\n", (unsigned)ESP.getFreeHeap());
    }

    moonshine_settings_server::moonshine_settings_server(const etl::webui::server_config_t& cfg)
        : etl::webui::server_setup(cfg)
    {
    }

    void moonshine_settings_server::stop_http_only()
    {
        Serial.println(F("[WiFiSetup] stopping HTTP server"));
        // Wi-Fi не трогаем: точка доступа остаётся поднятой
        if (m_server) {
            m_server->stop();
            m_server.reset();
        }
        m_initialized = false;
        Serial.printf("[WiFiSetup] HTTP server stopped, heap=%u\n", (unsigned)ESP.getFreeHeap());
    }

    void moonshine_settings_server::start_http_only()
    {
        Serial.printf("[WiFiSetup] starting HTTP server, heap=%u\n", (unsigned)ESP.getFreeHeap());
        start_http_server();
        m_initialized = true;
        Serial.printf("[WiFiSetup] HTTP server started, heap=%u\n", (unsigned)ESP.getFreeHeap());
    }

    bool moonshine_web_manager::start(bool settings_mode)
    {
        Serial.println(F("[WebManager] Starting..."));

        // Wi-Fi поднимается один раз: AP или подключение к локальной сети
        auto wifi = get_wifi_manager();
        if (auto cfg = etl::webui::settings::load_wifi_config(); cfg.has_value()) {
            wifi->update_config(cfg.value());
        }
        if (!wifi->begin()) {
            Serial.println(F("[WebManager] Wi-Fi initialization failed"));
            return false;
        }

        // manager::start_ap() не проверяет результат WiFi.softAP() и печатает успех всегда,
        // поэтому поднимаем точку доступа ещё раз и печатаем настоящий код возврата.
        // Если предыдущая попытка не удалась - эта её исправляет.
        if (!wifi->is_connected())
        {
            delay(100);
            auto cfg = etl::webui::settings::load_wifi_config();
            etl::webui::server_config_t ap_cfg = cfg.has_value() ? cfg.value() : make_default_config();

            bool ap_started = WiFi.softAP(ap_cfg.ap_ssid, ap_cfg.ap_password);
            Serial.printf("[WebManager] softAP(%s) = %d, mode = %d\n",
                          ap_cfg.ap_ssid, (int)ap_started, (int)WiFi.getMode());
            Serial.printf("[WebManager] AP IP = %s, MAC = %s\n",
                          WiFi.softAPIP().toString().c_str(), WiFi.macAddress().c_str());
        }

        ensure_servers();
        if (!m_content || !m_settings) {
            Serial.println(F("[WebManager] Server allocation failed"));
            return false;
        }

        if (settings_mode) {
            activate_settings();
        } else {
            activate_content();
        }

        return true;
    }

    void moonshine_web_manager::ensure_servers()
    {
        if (!m_content) {
            m_content = etl::make_shared<moonshine_web_server>();
            m_content->set_wifi_manager(get_wifi_manager());
            m_content->set_device_info(m_device_info);
            m_content->set_on_settings_callback(on_settings_requested);
            m_content->set_on_content_callback(on_content_requested);
        }

        if (!m_settings) {
            auto cfg = etl::webui::settings::load_wifi_config();
            m_settings = etl::make_shared<moonshine_settings_server>(
                cfg.has_value() ? cfg.value() : make_default_config());

            m_settings->set_wifi_manager(get_wifi_manager());
            m_settings->set_device_info(m_device_info);
            m_settings->set_on_settings_callback(on_settings_requested);
            m_settings->set_on_content_callback(on_content_requested);
        }
    }

    void moonshine_web_manager::activate_content()
    {
        Serial.println(F("[WebManager] activate_content()"));

        if (!m_content) {
            Serial.println(F("[WebManager] content server is not created"));
            return;
        }

        if (m_settings_active && m_settings) {
            Serial.println(F("[WebManager] stopping settings HTTP"));
            m_settings->stop_http_only();
            delay(100);   // дать сокету закрыться, прежде чем занять порт заново
        }

        Serial.println(F("[WebManager] loading settings"));
        m_content->load_settings();   // имя хоста, порт - из файловой системы
        m_content->start_http_only();

        m_settings_active = false;
        m_server = m_content;

        Serial.println(F("[WebManager] content server is active"));
    }

    void moonshine_web_manager::activate_settings()
    {
        Serial.println(F("[WebManager] activate_settings()"));

        if (!m_settings) {
            Serial.println(F("[WebManager] settings server is not created"));
            return;
        }

        if (!m_settings_active && m_content) {
            Serial.println(F("[WebManager] stopping content HTTP"));
            m_content->stop_http_only();
            delay(100);   // дать сокету закрыться, прежде чем занять порт заново
        }

        Serial.println(F("[WebManager] loading settings"));
        m_settings->load_settings();  // WiFi и настройки интерфейса - из файловой системы
        m_settings->start_http_only();

        m_settings_active = true;
        m_server = m_settings;

        Serial.println(F("[WebManager] settings server is active"));
    }

    etl::shared_ptr<etl::webui::web_server_base_t> moonshine_web_manager::on_create_content()
    {
        ensure_servers();
        return m_content;
    }

    etl::shared_ptr<etl::webui::web_server_base_t> moonshine_web_manager::on_create_settings()
    {
        ensure_servers();
        return m_settings;
    }

} // namespace webui
