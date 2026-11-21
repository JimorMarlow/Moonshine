# Moonshine — agent instructions

Rectification column controller on ESP32 (WROOM-38U): DS18B20 temperature sensors,
Hall-effect water flow sensors, web interface (page in PROGMEM), settings in LittleFS.
Firmware is built with PlatformIO + Arduino framework.

## Language

- This file (`AGENTS.md`) is kept **in English**: every addition or edit here is written in English.
- Commit messages are **in English** from v1.5.1 onwards. Russian is allowed only where it carries
  meaning — for example when quoting a Russian UI string or user-visible text. Older commits in the
  history are Russian; do not rewrite them.
- `README.md` and the rest of the project documentation stay **in Russian**, including the version
  history entries in `README.md`: a new version is described there in Russian, while the commit
  message for the same version is written in English.
- Comments in code and UI strings stay **in Russian**, as in the existing code.

## Build and verification

`pio` is not in PATH — call it by full path:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -e esp32-wroom-38u
```

- Before saying "done", build the project with this command and show the result (RAM/Flash, errors).
- Read compilation errors from the whole output: the first error is the cause, the rest are usually
  its consequences.
- Uploading and monitoring need a connected board — **ask the user first**:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -e esp32-wroom-38u -t upload
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" device monitor -b 115200
```

- Full clean: `... pio run -t clean`. Do not wipe `.pio` without a reason — rebuilds get slower.
- There are no unit tests; the correctness criterion is a successful build plus reading the code.

## What must not break

- `src/pinout.h` — the actual pin assignment. Do not change it without explicit user confirmation.
- `platformio.ini` — dependencies are pinned (ETL, Gyver libraries, LiquidCrystal_I2C).
  Add a new library only on request; do not bump versions "while at it".
- `src/version.h` — firmware version; `sync_version.py` copies it into `package.json` at build time.
  Bump PATCH for regular changes, MINOR for new features.
- Web UI (`src/web-ui.cpp`): the HTML page lives in PROGMEM (`HTML_PAGE`); keep large strings in
  PROGMEM and watch free RAM (the build prints it at the end).
- Never commit or push to git yourself. When a development stage is finished and the user asked to
  bump the version, prepare the commit message text and show it — the user commits after review.

## Web UI and setup mode (as implemented)

Two servers built on ETL base classes (`etl::webui`):

- **content server** (`webui::moonshine_web_server`) — normal mode: monitoring page and JSON;
- **settings server** (`etl::webui::server_setup`) — network, theme, font; created for the duration
  of setup and destroyed on return, so it does not hold memory permanently.

Switching is driven by `webui::moonshine_web_manager` (`etl::webui::web_manager`): `start_content()` /
`start_settings()` stop the current server and start a new one, while `on_create_content()` /
`on_create_settings()` create it.

1. Entering setup — `/api/settings` (⚙ button), three clicks of the hardware button, or holding the
   button at boot. The handler only schedules a callback (`schedule_settings_cb()`), our callback sets
   a flag, and the switch itself is done by `handle_pending_switch()` from `loop()`: a server must not
   be stopped from inside its own HTTP handler.
2. `start_settings()` calls `stop()` on the old server, and the base `web_server_base_t::stop()` runs
   `MDNS.end()`, stops the HTTP server, then `WiFi.disconnect(true)`, `WiFi.softAPdisconnect(true)`
   and `WiFi.mode(WIFI_OFF)` — **Wi-Fi goes down**. The new server brings it back in `begin()`: empty
   `wifi_ssid` → `Moonshine_AP` right away; non-empty → up to 10 s trying the local network, AP on
   failure. So switching to setup restarts the access point (same SSID, 1–2 s gap; up to 10 s when the
   saved network is unreachable) — it does not stay continuous.
3. The switch page does not do a single redirect: it polls `/` every 2 s until the new server answers.
4. There is one ETL Wi-Fi manager for the whole run (`set_wifi_manager()`). If each server created its
   own, the destructor of the old server would call `manager::~manager()` → `stop()` →
   `WiFi.mode(WIFI_OFF)` after the new server had started, killing its access point.
5. Returning without saving — `/api/back`: on ESP32 a content callback is scheduled and the manager
   switches back the same way (Wi-Fi restarts again); on ESP8266 the library reboots the controller
   instead.
6. Saving — `/api/save`: Wi-Fi settings (`/settings/wifi.cfg`) and UI settings (`/settings/ui.cfg`)
   are written to LittleFS, the client gets the response, then `ESP.restart()`. `/api/reset` restores
   defaults and also reboots.
7. After a reboot `setup()` calls `start_web_ui(button_held)`: settings are read from LittleFS and
   either the settings server (button held) or the web UI starts. Then `manager::begin()`: non-empty
   `wifi_ssid` → connect to the local network, and on success `WiFi.mode(WIFI_AP_STA)`, but `softAP()`
   is not called — there is no access point, access is only through the address on the local network;
   otherwise AP mode.
8. mDNS `moonshine.local` comes up in both AP and STA; the `http` service is registered by the Wi-Fi
   manager itself, our servers must not add it.

If a gap-free AP is ever needed, `stop()` must be overridden in both servers: shut down only the HTTP
server and mDNS, leaving Wi-Fi alone.

## Style

- Do not reformat other people's files wholesale: minimal diff, same style, same names.
- Comments in code — to the point and in Russian; do not comment the obvious.
- Do not touch `.pio/`, `docs/`, `README.md` without a reason.
- Commits are short, in English, in the style of the history (`v1.5.1 fix setup server switch`).
  Russian words are fine only where they carry meaning (for example, a quoted Russian UI string).

## Workflow

1. Read the affected files (and `README.md`) first, only then edit.
2. Change one logical thing at a time, and build after each change.
3. Use subagents for code search and for reviewing others' changes, but make the edits yourself.
4. If a build fails for an environment reason (no toolchain, no board), say so directly; do not
   present it as a code bug.
