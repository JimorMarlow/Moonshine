# Moonshine — agent instructions

Rectification column controller on ESP32 (WROOM-38U): DS18B20 temperature sensors,
Hall-effect water flow sensors, web interface (page in PROGMEM), settings in LittleFS.
Firmware is built with PlatformIO + Arduino framework.

## Canary

Owner of this project: the value of `git config user.name` (currently `JimorMarlow`). Take it
from git yourself, do not ask. Start **every** answer with the owner's name and a comma, then
continue the answer on the same line — so it is visible that this file was read and the rule is
followed:

`JimorMarlow, <the answer>`

Even a one-line answer, an error report or a question starts with it.

## No STL — extend ETL instead

- Do not use STL (`std::vector`, `std::string`, ...) in this project: only `etl::`, no exceptions,
  no RTTI, no unnecessary dynamic allocations.
- If ETL lacks a common tool that the task needs, do not substitute `std::` and do not leave an
  ad-hoc workaround in the project: implement it here in an `etl_*` file, debug it on the device,
  and when it is finished propose it to the ETL developer for inclusion in the ETL library.

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

Two servers built on ETL base classes (`etl::webui`), both created once at boot and kept alive for
the whole run:

- **content server** (`webui::moonshine_web_server`) — normal mode: monitoring page and JSON;
- **settings server** (`webui::moonshine_settings_server`) — subclass of `etl::webui::server_setup`
  (network, theme, font) that can start and stop only its HTTP part.

Switching is done by `webui::moonshine_web_manager`, which deliberately does **not** use the base
`web_manager::start_content()` / `start_settings()`: those re-initialise Wi-Fi.

1. `start(settings_mode)` runs once at boot: loads the Wi-Fi config from LittleFS, brings Wi-Fi up
   through the single ETL manager (`get_wifi_manager()`), creates both servers, activates one.
2. Entering setup — `/api/settings` (⚙ button), three clicks of the hardware button, or holding the
   button at boot. The handler only schedules a callback (`schedule_settings_cb()`), our callback sets
   a flag, and `handle_pending_switch()` from `loop()` performs the switch: a server must not be
   stopped from inside its own HTTP handler.
3. `activate_settings()` / `activate_content()` stop only the HTTP server of the previous mode
   (`stop_http_only()`), wait 20 ms for the socket, reload settings from LittleFS and start the new
   HTTP server (`start_http_only()`). **Wi-Fi is never touched**: the access point stays up and the
   client stays associated — only the TCP connection to port 80 is dropped.
4. The switch page polls `/` every second until the new server answers, then navigates.
5. Returning without saving — `/api/back`: the library schedules a content callback, our flag is set,
   and `activate_content()` runs the same non-disruptive switch.
6. Saving — `/api/save`: Wi-Fi (`/settings/wifi.cfg`) and UI (`/settings/ui.cfg`) settings are written
   to LittleFS, the client gets the response, then `ESP.restart()`. `/api/reset` restores defaults and
   reboots as well.
7. After a reboot `setup()` calls `start_web_ui(button_held)`: settings are read from LittleFS and
   either the settings server (button held) or the web UI starts. Wi-Fi mode: empty `wifi_ssid` →
   `Moonshine_AP`; non-empty → connect to the local network, and on success `WiFi.mode(WIFI_AP_STA)`
   without calling `softAP()` (no access point, only the LAN address); on failure AP mode.
8. mDNS `moonshine.local` comes up in both AP and STA; the `http` service is registered by the Wi-Fi
   manager itself — our servers must not add it.

Never use `web_server_base_t::stop()` for switching: it turns Wi-Fi off (`WiFi.mode(WIFI_OFF)`),
which kills the access point. Use the `*_http_only()` helpers instead.

### Interface settings on the content page

`ui_config` (language, dark theme, large font, bold values) is applied by the content page itself:

- `handle_root()` substitutes `${LANG}` and `${UI_CLASS}` (`dark-theme`, `large-font`,
  `bold-values`) into the page from `get_ui_config()`;
- texts are translated in JS with the `TR` dictionary and the `tr(key)` helper, applied through
  `data-i18n` attributes; the helper is named `tr` because the minified code already uses `t` as a
  local variable;
- dark theme colours mirror the ETL setup page.

Never translate: the `Moonshine` name, schematic labels (`Steam`, `Heater`, `Deflegmator`,
`Condenser`, `Product`) and sensor designations (`Steam`, `DF.T`, `Cool.T`, `Heater`, `DF.Flow`,
`Cool.Flow`). Headings, descriptions and status/error messages must be translated.

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
