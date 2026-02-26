/**
 * @file web-ui.cpp
 * @brief Реализация веб-сервера Moonshine для ESP8266
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
    static const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
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
        .bold-mode .temp-value,.bold-mode .flow-value,.bold-mode .temp-text,.bold-mode .flow-text,.bold-mode .uptime-value{font-weight:700}
        @media(max-width:900px){.main-panel{grid-template-columns:1fr;grid-template-rows:auto auto;height:auto}.scheme-container{height:auto;min-height:480px;aspect-ratio:530/480}.sensors-panel{height:auto;max-height:none;overflow:visible}.sensors-content{overflow-y:visible}.bold-btn{width:40px;height:40px;font-size:20px}.uptime-value{font-size:24px}}
    </style>
</head>
<body>
    <div class="container">
        <div class="main-panel">
            <div class="scheme-container">
                <div class="internal-header">Moonshine v${VERSION}</div>
                <div id="statusBar" class="status-bar status-ok">
                    <span class="status-icon" id="statusIcon">✓</span>
                    <span id="statusText">Система работает нормально</span>
                </div>
                <svg class="distillation-svg" viewBox="0 0 530 480" xmlns="http://www.w3.org/2000/svg" preserveAspectRatio="xMidYMid meet">
                    <defs>
                        <linearGradient id="columnGradient" x1="0%" y1="100%" x2="0%" y2="0%"><stop offset="0%" stop-color="#e74c3c" id="colStopLow"/><stop offset="100%" stop-color="#f39c12" id="colStopHigh"/></linearGradient>
                        <linearGradient id="vaporGradient" x1="0%" y1="0%" x2="100%" y2="0%"><stop offset="0%" stop-color="#e67e22" id="vapStopLow"/><stop offset="100%" stop-color="#3498db" id="vapStopHigh"/></linearGradient>
                        <linearGradient id="dfOutGradient" x1="0%" y1="100%" x2="0%" y2="0%"><stop offset="0%" stop-color="#3498db" id="dfOutStopLow"/><stop offset="100%" stop-color="#f39c12" id="dfOutStopHigh"/></linearGradient>
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
                    <text x="225" y="402" text-anchor="middle" fill="#2c3e50" font-size="14" font-weight="500">Heater</text>
                    <rect x="185" y="354" width="85" height="32" rx="3" class="temp-rect" id="rectHeater"/>
                    <text x="227.5" y="370" class="temp-text" id="valueHeaterNum">--.-°</text>
                    <rect x="195" y="405" width="60" height="12" rx="3" fill="#e74c3c" opacity="0.3"/>
                    <rect x="185" y="37" width="85" height="32" rx="3" class="temp-rect" id="rectSteam"/>
                    <text x="227.5" y="53" class="temp-text" id="valueSteamNum">--.-°</text>
                    <text x="225" y="32" text-anchor="middle" class="temp-label">Steam</text>
                    <path d="M 225 110 L 225 80 L 315 80 L 315 110" class="vapor-pipe" stroke="url(#vaporGradient)" fill="none"/>
                    <path d="M 315 200 L 315 240" class="product-pipe" id="productPipe"/>
                    <text x="315" y="255" text-anchor="middle" fill="#7f8c8d" font-size="10">Product</text>
                    <g id="dropIndicator" transform="translate(315,270)" style="display:none"><use href="#dropShape"/></g>
                    <rect x="185" y="110" width="80" height="90" rx="5" class="deflegmator" id="deflegmator"/>
                    <text x="225" y="155" text-anchor="middle" fill="#2c3e50" font-size="10">Deflegmator</text>
                    <rect x="210" y="120" width="30" height="70" rx="3" fill="#27ae60" opacity="0.2"/>
                    <rect x="275" y="110" width="80" height="90" rx="5" class="condenser" id="condenser"/>
                    <text x="315" y="155" text-anchor="middle" fill="#2c3e50" font-size="10">Condenser</text>
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
                        <button class="bold-btn" id="boldBtn" title="Жирный шрифт для цифр">B</button>
                    </div>
                    <div class="section-title">Температура</div>
                    <div class="sensor-card temp-sensor"><div class="sensor-title">Steam - Верх колонны</div><div class="temp-display"><div class="temp-value" id="cardValueSteam">--.-</div><div class="temp-unit">°C</div></div></div>
                    <div class="sensor-card temp-sensor"><div class="sensor-title">DF.T - Дефлегматор (выход)</div><div class="temp-display"><div class="temp-value" id="cardValueDFT">--.-</div><div class="temp-unit">°C</div></div></div>
                    <div class="sensor-card temp-sensor"><div class="sensor-title">Cool.T - Конденсер (выход)</div><div class="temp-display"><div class="temp-value" id="cardValueCoolT">--.-</div><div class="temp-unit">°C</div></div></div>
                    <div class="sensor-card temp-sensor"><div class="sensor-title">Heater - Температура в баке</div><div class="temp-display"><div class="temp-value" id="cardValueHeater">--.-</div><div class="temp-unit">°C</div></div></div>
                    <div class="section-title" style="margin-top:20px">Поток воды</div>
                    <div class="sensor-card flow-sensor"><div class="sensor-title">DF.Flow - Дефлегматор</div><div style="display:flex;align-items:baseline"><div class="flow-value" id="cardValueDFFlow">0</div><div class="flow-unit">ml/min</div></div></div>
                    <div class="sensor-card flow-sensor"><div class="sensor-title">Cool.Flow - Конденсер</div><div style="display:flex;align-items:baseline"><div class="flow-value" id="cardValueCoolFlow">0</div><div class="flow-unit">ml/min</div></div></div>
                    <div class="update-time">Обновлено: <span id="lastUpdate">--:--:--</span></div>
                </div>
            </div>
        </div>
    </div>
    <script>
        const COLOR_BLENDER=[{temp:15,color:"#2980b9"},{temp:20,color:"#3498db"},{temp:25,color:"#f1c40f"},{temp:28,color:"#e67e22"},{temp:30,color:"#e74c3c"},{temp:35,color:"#c0392b"}];
        let boldMode=false;
        function toggleBoldMode(){boldMode=!boldMode;const e=document.getElementById("boldBtn"),t=document.querySelector(".container");boldMode?(e.classList.add("active"),t.classList.add("bold-mode")):(e.classList.remove("active"),t.classList.remove("bold-mode"))}
        document.getElementById("boldBtn").addEventListener("click",toggleBoldMode);
        let lastUpdateTime=Date.now(),connectionState="connected";
        function updateConnectionState(){const e=Date.now(),t=(e-lastUpdateTime)/1e3,n=document.getElementById("statusBar"),o=document.getElementById("statusIcon"),r=document.getElementById("statusText");t>=20?(connectionState="error",n.className="status-bar status-error",o.textContent="✕",r.textContent="Устройство не отвечает"):t>=10?(connectionState="warning",n.className="status-bar status-warning",o.textContent="⚠",r.textContent="Устройство не отвечает"):connectionState="connected"}
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
        function checkRules(e){const t=[],{Heater:n,CoolFlow:o,CoolT:r,DFT:s,DFFlow:a}=e;n.value>55&&0===o.value&&t.push({type:"warning",text:"Включите подачу воды в охладитель"}),r.value>30&&(0===o.value?t.push({type:"error",text:"Нет воды в охладителе!"}):t.push({type:"warning",text:"Добавьте воды в охладитель"})),s.value>30&&(0===a.value?t.push({type:"error",text:"Нет воды в дефлегматоре!"}):t.push({type:"warning",text:"Добавьте воды в дефлегматор"}));return 0===t.length&&t.push({type:"ok",text:"Система работает нормально"}),t}
        function updateStatusBar(e){if(connectionState!=="connected")return;const t=checkRules(e),n=document.getElementById("statusBar"),o=document.getElementById("statusIcon"),r=document.getElementById("statusText"),s={error:3,warning:2,ok:1},a=t.reduce(((e,t)=>s[t.type]>s[e.type]?t:e));n.className="status-bar",n.classList.add(`status-${a.type}`),o.textContent="ok"===a.type?"✓":"warning"===a.type?"⚠":"✕",r.textContent=a.text}
        function updateSensors(e){
            if(connectionState!=="connected")return;
            document.getElementById("valueHeaterNum").textContent=e.heater_temperature?formatTemp(e.heater_temperature)+"°":"--.-°";
            document.getElementById("valueSteamNum").textContent=e.steam_temperature?formatTemp(e.steam_temperature)+"°":"--.-°";
            document.getElementById("valueDFTNum").textContent=e.deflegmater_temperature?formatTemp(e.deflegmater_temperature)+"°":"--.-°";
            document.getElementById("valueCoolTNum").textContent=e.condenser_temperature?formatTemp(e.condenser_temperature)+"°":"--.-°";
            document.getElementById("cardValueHeater").textContent=e.heater_temperature?formatTemp(e.heater_temperature):"--.-";
            document.getElementById("cardValueSteam").textContent=e.steam_temperature?formatTemp(e.steam_temperature):"--.-";
            document.getElementById("cardValueDFT").textContent=e.deflegmater_temperature?formatTemp(e.deflegmater_temperature):"--.-";
            document.getElementById("cardValueCoolT").textContent=e.condenser_temperature?formatTemp(e.condenser_temperature):"--.-";
            document.getElementById("cardValueDFFlow").textContent=e.deflegmater_water_flow.toFixed(0);
            document.getElementById("cardValueCoolFlow").textContent=e.condenser_water_flow.toFixed(0);
            document.getElementById("svgValueDFFlow").textContent=e.deflegmater_water_flow.toFixed(0);
            document.getElementById("svgValueCoolFlow").textContent=e.condenser_water_flow.toFixed(0);
            document.getElementById("cardValueUptime").textContent=formatUptime(e.uptime_ms);
            ["DFFlow","CoolFlow"].forEach((t=>{const n=e[t.toLowerCase().replace(".","_")],o=document.getElementById("wheel"+t.replace(".","")+"Group"),r=document.getElementById("warning"+t.replace(".",""));if(o&&r){if(n<10)o.style.display="none",r.style.display="block";else{o.style.display="block",r.style.display="none";const s=document.getElementById(t.replace(".","")+"WaterInPipe");s&&(s.classList.remove("flow-slow","flow-medium","flow-fast"),s.classList.add(getFlowPipeClass(n)),s.style.strokeDasharray=""),o.classList.remove("flow-wheel-slow","flow-wheel-medium","flow-wheel-fast"),o.classList.add(getFlowWheelClass(n))}}}));
            const t=document.getElementById("dfWaterOutPipe");t&&(t.style.stroke=getTempColor(e.deflegmater_temperature||20),e.deflegmater_water_flow>=10?(t.classList.remove("flow-slow","flow-medium","flow-fast"),t.classList.add(getFlowPipeClass(e.deflegmater_water_flow)),t.style.strokeDasharray=""):(t.classList.remove("flow-slow","flow-medium","flow-fast"),t.style.strokeDasharray="none"));
            const n=document.getElementById("coolWaterOutPipe");n&&(n.style.stroke=getTempColor(e.condenser_temperature||20),e.condenser_water_flow>=10?(n.classList.remove("flow-slow","flow-medium","flow-fast"),n.classList.add(getFlowPipeClass(e.condenser_water_flow)),n.style.strokeDasharray=""):(n.classList.remove("flow-slow","flow-medium","flow-fast"),n.style.strokeDasharray="none"));
            const o=getColumnColors(e.heater_temperature||20,e.steam_temperature||20);document.getElementById("colStopLow").setAttribute("stop-color",o.low),document.getElementById("colStopHigh").setAttribute("stop-color",o.high);
            const r=getTempColor((e.steam_temperature||20)>35?35:(e.steam_temperature||20)),s=getTempColor(e.condenser_temperature||20);document.getElementById("vapStopLow").setAttribute("stop-color",r),document.getElementById("vapStopHigh").setAttribute("stop-color",s);
            const a=document.getElementById("heater");a&&(a.style.stroke=getTempColor(e.heater_temperature||20));
            updateDropIndicator(e.steam_temperature||20);
            updateHeaterBubbles(e.heater_temperature||20);
            updateStatusBar({Heater:{value:e.heater_temperature||20},CoolFlow:{value:e.condenser_water_flow},CoolT:{value:e.condenser_temperature||20},DFT:{value:e.deflegmater_temperature||20},DFFlow:{value:e.deflegmater_water_flow}});
            document.getElementById("lastUpdate").textContent=new Date().toLocaleTimeString("ru-RU");
        }
        function fetchState(){fetch("/api/state").then((e=>e.json())).then((e=>{lastUpdateTime=Date.now();updateSensors(e)})).catch((e=>{console.error("Error fetching state:",e)}));}
        fetchState();
        setInterval(fetchState,500);
    </script>
</body>
</html>
)rawliteral";

    MoonshineWebServer server;

    MoonshineWebServer::MoonshineWebServer(const config_t& cfg)
        : m_config(cfg)
        , m_server(m_config.port)
    {
    }

    void MoonshineWebServer::setConfig(const config_t& cfg)
    {
        m_config = cfg;
    }

    bool MoonshineWebServer::begin()
    {
        Serial.println(F("[WebUI] Initializing..."));

        // Настройка WiFi
        if (m_config.wifi_ssid && strlen(m_config.wifi_ssid) > 0) {
            // Режим STA+AP или только STA
            Serial.print(F("[WebUI] Connecting to WiFi: "));
            Serial.println(m_config.wifi_ssid);

            WiFi.hostname(m_config.hostname);
            WiFi.mode(WIFI_STA);
            WiFi.begin(m_config.wifi_ssid, m_config.wifi_password);

            uint8_t attempts = 30;
            while (WiFi.status() != WL_CONNECTED && attempts > 0) {
                delay(500);
                Serial.print(F("."));
                attempts--;
            }

            if (WiFi.status() == WL_CONNECTED) {
                Serial.println(F("\n[WebUI] Connected to WiFi"));
                Serial.print(F("[WebUI] IP: "));
                Serial.println(WiFi.localIP());
            } else {
                Serial.println(F("\n[WebUI] WiFi connection failed, falling back to AP"));
                WiFi.mode(WIFI_AP);
                WiFi.softAP(m_config.ap_ssid, m_config.ap_password);
                Serial.print(F("[WebUI] AP IP: "));
                Serial.println(WiFi.softAPIP());
            }
        } else {
            // Только AP режим
            Serial.println(F("[WebUI] Starting in AP mode"));
            WiFi.mode(WIFI_AP);
            WiFi.softAP(m_config.ap_ssid, m_config.ap_password);
            Serial.print(F("[WebUI] AP IP: "));
            Serial.println(WiFi.softAPIP());
        }

        // mDNS
        if (MDNS.begin(m_config.hostname)) {
            Serial.print(F("[WebUI] mDNS: http://"));
            Serial.print(m_config.hostname);
            Serial.println(F(".local"));
            MDNS.addService("http", "tcp", m_config.port);
        }

        // Регистрация обработчиков
        m_server.on("/", HTTP_GET, std::bind(&MoonshineWebServer::handleRoot, this));
        m_server.on("/api/state", HTTP_GET, std::bind(&MoonshineWebServer::handleApiState, this));
        m_server.on("/api/status", HTTP_GET, std::bind(&MoonshineWebServer::handleApiStatus, this));
        m_server.onNotFound([]() {
            Serial.println(F("[WebUI] 404 Not Found"));
        });

        m_server.begin();
        m_initialized = true;

        Serial.println(F("[WebUI] Server started"));
        return true;
    }

    void MoonshineWebServer::handleClient()
    {
        if (m_initialized) {
            MDNS.update();
            m_server.handleClient();
        }
    }

    bool MoonshineWebServer::isConnected() const
    {
        return WiFi.status() == WL_CONNECTED || WiFi.getMode() == WIFI_AP;
    }

    String MoonshineWebServer::getIPAddress() const
    {
        if (WiFi.status() == WL_CONNECTED) {
            return WiFi.localIP().toString();
        }
        return WiFi.softAPIP().toString();
    }

    String MoonshineWebServer::getMode() const
    {
        WiFiMode_t mode = WiFi.getMode();
        switch (mode) {
            case WIFI_STA: return "STA";
            case WIFI_AP: return "AP";
            case WIFI_AP_STA: return "STA+AP";
            default: return "UNKNOWN";
        }
    }

    void MoonshineWebServer::setStateGetter(settings::moonshine::state_t (*getter)())
    {
        m_stateGetter = getter;
    }

    void MoonshineWebServer::handleRoot()
    {
        Serial.println(F("[WebUI] Serving HTML page"));

        // Замена версии в HTML
        String html = FPSTR(HTML_PAGE);
        html.replace("${VERSION}", MS_VERSION_STRING);

        m_server.sendHeader("Content-Type", "text/html");
        m_server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        m_server.send(200, "text/html", html);
    }

    void MoonshineWebServer::handleApiState()
    {
        Serial.println(F("[WebUI] API: /api/state"));

        if (m_stateGetter) {
            settings::moonshine::state_t state = m_stateGetter();
            String json = stateToJson(state);

            m_server.sendHeader("Content-Type", "application/json");
            m_server.sendHeader("Cache-Control", "no-cache");
            m_server.send(200, "application/json", json);
        } else {
            m_server.send(500, "application/json", F("{\"error\":\"State getter not set\"}"));
        }
    }

    void MoonshineWebServer::handleApiStatus()
    {
        String json = String(F("{\"uptime_ms\":")) + millis() +
                      String(F(",\"mode\":\"")) + getMode() +
                      String(F("\",\"ip\":\"")) + getIPAddress() +
                      String(F("\",\"hostname\":\"")) + m_config.hostname +
                      String(F("\"}"));

        m_server.sendHeader("Content-Type", "application/json");
        m_server.send(200, "application/json", json);
    }

    String MoonshineWebServer::stateToJson(const settings::moonshine::state_t& state)
    {
        String json = String(F("{"));
        json += String(F("\"uptime_ms\":")) + String(state.uptime_ms) + String(F(","));

        // Steam temperature
        json += String(F("\"steam_temperature\":"));
        if (state.steam_temperature) {
            json += String(*state.steam_temperature, 1);
        } else {
            json += "null";
        }
        json += ",";

        // Heater temperature
        json += String(F("\"heater_temperature\":"));
        if (state.heater_temperature) {
            json += String(*state.heater_temperature, 1);
        } else {
            json += "null";
        }
        json += ",";

        // Deflegmator temperature
        json += String(F("\"deflegmater_temperature\":"));
        if (state.deflegmater_temperature) {
            json += String(*state.deflegmater_temperature, 1);
        } else {
            json += "null";
        }
        json += ",";

        // Condenser temperature
        json += String(F("\"condenser_temperature\":"));
        if (state.condenser_temperature) {
            json += String(*state.condenser_temperature, 1);
        } else {
            json += "null";
        }
        json += ",";

        // Flow rates
        json += String(F("\"deflegmater_water_flow\":")) + String(state.deflegmater_water_flow, 1) + String(F(","));
        json += String(F("\"condenser_water_flow\":")) + String(state.condenser_water_flow, 1);

        json += String(F("}"));
        return json;
    }

} // namespace webui
