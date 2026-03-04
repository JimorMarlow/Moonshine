# 🔐 Управление конфиденциальными данными в Arduino-проектах

> **Теги:** #Arduino #ESP8266 #ESP32 #Git #Security #Secrets #BestPractices

## 📋 Содержание

1. [Проблема](#-проблема)
2. [Ключевая идея](#-ключевая-идея-разделение-кода-и-данных)
3. [Архитектура решения](#-архитектура-решения)
4. [Файл-шаблон (secrets_template.h)](#-файл-шаблон-secrets_templateh)
5. [Реальный файл (secrets.h)](#-реальный-файл-secretsh)
6. [Использование в коде](#-использование-в-основном-коде-maincpp)
7. [Настройка .gitignore](#-настройка-gitignore)
8. [Синхронизация между рабочими местами](#-синхронизация-между-рабочими-местами)
9. [Продвинутые техники](#-продвинутые-техники)
10. [Практический пример для Moonshine](#-практический-пример-для-вашего-moonshine-проекта)
11. [Итоговый алгоритм работы](#-итоговый-алгоритм-работы)
12. [Бонус: Работа с GitHub Actions](#-бонус-работа-с-github-actions)

---

## 🎯 Проблема

У вас есть публичные проекты на GitHub, использующие общую кодовую базу (`etl`). Вам нужно:
- Хранить **пароли, токены API, SSID** в безопасности
- **Переключаться** между разными рабочими местами (дом, мастерская, клиент)
- **Синхронизировать** приватные данные между устройствами
- **Не допустить** случайной публикации секретов

---

## 💡 Ключевая идея: разделение кода и данных

**Конфиденциальные данные никогда не должны попадать в публичные репозитории.**  
Вместо этого вы храните их локально, а в проектах используете **заглушки**.

---

## 🏗 Архитектура решения

```
Ваш проект (например, moonshine-controller/)
├── src/
│   ├── main.cpp
│   ├── secrets.h              // ← В .gitignore, содержит реальные данные
│   └── secrets_template.h     // ← В репозитории, содержит заглушки
├── .gitignore
└── platformio.ini
```


---

## 📄 Файл-шаблон (`secrets_template.h`)

Этот файл **хранится в репозитории** и содержит заглушки.  
Новый разработчик копирует его в `secrets.h` и заполняет своими данными.

```cpp
#pragma once
// ===================================================
// ШАБЛОН ДЛЯ СЕКРЕТНЫХ ДАННЫХ
// Скопируйте этот файл как secrets.h и заполните свои данные
// ===================================================

// WiFi сети для разных локаций
struct WiFiCredentials {
    const char* ssid;
    const char* password;
};

// Конфигурация для разных мест работы
namespace Locations {
    // Домашняя сеть
    inline constexpr WiFiCredentials HOME = {
        .ssid = "your_home_ssid",
        .password = "your_home_password"
    };
    
    // Сеть в мастерской
    inline constexpr WiFiCredentials WORKSHOP = {
        .ssid = "workshop_wifi",
        .password = "workshop_pass"
    };
    
    // Мобильный хотспот
    inline constexpr WiFiCredentials MOBILE = {
        .ssid = "mobile_hotspot",
        .password = "hotspot_password"
    };
}

// API ключи и токены
namespace API {
    inline constexpr const char* TELEGRAM_BOT_TOKEN = "your_telegram_bot_token";
    inline constexpr const char* TELEGRAM_CHAT_ID = "your_chat_id";
    inline constexpr const char* THINGSPEAK_API_KEY = "your_thingspeak_key";
    inline constexpr const char* MQTT_SERVER = "mqtt.your-server.com";
    inline constexpr const char* MQTT_USER = "mqtt_user";
    inline constexpr const char* MQTT_PASS = "mqtt_password";
}

// Выбор активной конфигурации
// Измените эту строку для переключения между локациями
#define ACTIVE_LOCATION Locations::HOME
```

---

## 🔒 Реальный файл (`secrets.h`)

Этот файл никогда не попадает в Git. Он создаётся локально на каждом рабочем месте.

```cpp
#pragma once
// ===================================================
// РЕАЛЬНЫЕ ДАННЫЕ - НЕ В COMMIT!
// ===================================================

namespace Locations {
    inline constexpr WiFiCredentials HOME = {
        .ssid = "MyHomeWiFi",
        .password = "SuperSecretPass123"
    };

    inline constexpr WiFiCredentials WORKSHOP = {
        .ssid = "WorkshopNet",
        .password = "WorkshopPass456"
    };

    inline constexpr WiFiCredentials MOBILE = {
        .ssid = "iPhone",
        .password = "12345678"
    };
}

namespace API {
    inline constexpr const char* TELEGRAM_BOT_TOKEN = "1234567890:ABCdefGHIjklMNOpqrsTUVwxyz";
    inline constexpr const char* TELEGRAM_CHAT_ID = "-123456789";
    inline constexpr const char* THINGSPEAK_API_KEY = "ABCDEFGHIJKLMNOP";
    inline constexpr const char* MQTT_SERVER = "mqtt.cloud.com";
    inline constexpr const char* MQTT_USER = "device_42";
    inline constexpr const char* MQTT_PASS = "mqtt_secret_2026";
}

#define ACTIVE_LOCATION Locations::HOME
```

---

## 🧠 Использование в основном коде (`main.cpp`)

```cpp
#include <Arduino.h>
#include "secrets.h"  // Подключаем секреты

void setup() {
    Serial.begin(115200);

    // Используем активную конфигурацию
    WiFi.begin(ACTIVE_LOCATION.ssid, ACTIVE_LOCATION.password);

    // Или явно выбираем конкретную
    // WiFi.begin(Locations::HOME.ssid, Locations::HOME.password);

    // Используем API ключи
    sendToTelegram(API::TELEGRAM_BOT_TOKEN, API::TELEGRAM_CHAT_ID, "Hello!");
}
```

---

## 🚫 Настройка `.gitignore`

```gitignore
# Игнорируем файлы с реальными данными
secrets.h
private/
*.private.*
*_secret.h
*_credentials.h

# Игнорируем локальные конфиги IDE
.vscode/
.idea/
*.swp
*.swo

# Игнорируем временные файлы
*.tmp
*.bak
*.log
```

---

## 🔄 Синхронизация между рабочими местами

### Вариант А: Приватный репозиторий для секретов (РЕКОМЕНДУЕТСЯ)

Создайте отдельный приватный репозиторий:

```
Приватный репозиторий: github.com/JimorMarlow/private-secrets
Структура:
├── moonshine/
│   └── secrets.h
├── project2/
│   └── secrets.h
└── sync.sh
```

#### Скрипт синхронизации (`sync.sh`):

```bash
#!/bin/bash
# Синхронизирует секреты во все проекты

PRIVATE_SECRETS_DIR=~/projects/private-secrets
PROJECTS_DIR=~/projects

# Копируем секреты в каждый проект
cp $PRIVATE_SECRETS_DIR/moonshine/secrets.h \
   $PROJECTS_DIR/moonshine-controller/src/

cp $PRIVATE_SECRETS_DIR/other-project/secrets.h \
   $PROJECTS_DIR/other-project/src/

echo "✅ Secrets synchronized!"
```

### Вариант Б: Ручная синхронизация

```bash
# На новом рабочем месте клонируем репозиторий
git clone https://github.com/JimorMarlow/moonshine-controller.git

# Копируем secrets.h через защищенный канал
# - USB-флешка
# - Signal / Telegram (self-destruct)
# - Зашифрованный email (GPG)
```

### Вариант В: Использование переменных окружения (для CI/CD)

```ini
; platformio.ini
[env:nodemcuv2]
platform = espressif8266
board = nodemcuv2
framework = arduino
build_flags =
    -DWIFI_SSID="${WIFI_SSID}"
    -DWIFI_PASS="${WIFI_PASS}"
    -DTELEGRAM_TOKEN="${TELEGRAM_TOKEN}"
```

---

## 🚀 Продвинутые техники

### 1. Множественные конфигурации с переключателем

```cpp
// secrets.h
enum class Location {
    HOME,
    WORK,
    COFFEE_SHOP,
    CLIENT_SITE
};

// Выбираем одним переключателем
constexpr Location CURRENT_LOCATION = Location::HOME;

// Автоматический выбор SSID
const char* getWiFiSSID() {
    switch(CURRENT_LOCATION) {
        case Location::HOME: return "HomeWiFi";
        case Location::WORK: return "OfficeNet";
        case Location::COFFEE_SHOP: return "CoffeeWiFi";
        case Location::CLIENT_SITE: return "ClientCorp";
        default: return "Default";
    }
}
```

### 2. Использование ETL для хранения конфигурации

```cpp
#include <etl/array.h>
#include <etl/string.h>

struct LocationConfig {
    etl::string<32> ssid;
    etl::string<64> password;
    etl::string<16> mqtt_server;
    uint16_t mqtt_port;
};

constexpr etl::array<LocationConfig, 3> CONFIGS = {{
    {"HomeWiFi", "homepass123", "mqtt.home", 1883},
    {"OfficeNet", "office456", "mqtt.office", 1883},
    {"Mobile", "mobile789", "mqtt.duckdns.org", 8883}
}};

// Использование
WiFi.begin(CONFIGS[0].ssid.c_str(), CONFIGS[0].password.c_str());
```

### 3. Шифрование секретов (GPG)

```bash
# Создание зашифрованного файла
gpg -c secrets.h  # создаст secrets.h.gpg

# Расшифровка на новом месте
gpg secrets.h.gpg  # запросит пароль
```

### 4. Автоматическое определение локации по MAC-адресу

```cpp
String getCurrentLocation() {
    // Пытаемся подключиться к известным сетям
    if (testWiFi("HomeWiFi")) return "HOME";
    if (testWiFi("OfficeNet")) return "WORK";
    return "UNKNOWN";
}
```

---

## 🥃 Практический пример для вашего Moonshine проекта

### `src/secrets_template.h`:

```cpp
#pragma once
// ============ TEMPLATE - COPY TO secrets.h ============

// WiFi для разных локаций
#define WIFI_HOME_SSID     "your_home_ssid"
#define WIFI_HOME_PASS     "your_home_password"
#define WIFI_OFFICE_SSID   "office_ssid"
#define WIFI_OFFICE_PASS   "office_password"

// API ключи
#define TELEGRAM_BOT_TOKEN "your_telegram_token"
#define THINGSPEAK_API_KEY "your_thingspeak_key"
#define THINGSPEAK_CHANNEL "your_channel_id"

// Blynk / MQTT
#define BLYNK_AUTH_TOKEN   "your_blynk_token"
#define MQTT_SERVER        "mqtt.your-server.com"
#define MQTT_PORT          1883

// Выберите локацию: HOME, OFFICE
#define ACTIVE_LOCATION HOME
```

### `src/main.cpp`:

```cpp
#include "secrets.h"

#if ACTIVE_LOCATION == HOME
  #define WIFI_SSID WIFI_HOME_SSID
  #define WIFI_PASS WIFI_HOME_PASS
#elif ACTIVE_LOCATION == OFFICE
  #define WIFI_SSID WIFI_OFFICE_SSID
  #define WIFI_PASS WIFI_OFFICE_PASS
#else
  #error "Unknown location"
#endif

void connectWiFi() {
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n✅ Connected!");
}

void sendTelegramAlert(const char* message) {
    // Используем TELEGRAM_BOT_TOKEN из secrets.h
    // ...
}
```

---

## 📋 Итоговый алгоритм работы

### При создании нового проекта:

1. Создаёте `secrets_template.h` с заглушками
2. Добавляете `secrets.h` в `.gitignore`
3. Делаете первый коммит с шаблоном

### На каждом рабочем месте:

```bash
git clone https://github.com/JimorMarlow/project.git
cp src/secrets_template.h src/secrets.h
# Редактируете secrets.h, вставляя реальные данные
```

### При переключении между местами:

```bash
# Вариант 1: Меняете ACTIVE_LOCATION в secrets.h
#define ACTIVE_LOCATION WORKSHOP

# Вариант 2: Копируете готовый secrets.h из приватного репозитория
cp ~/private-secrets/project/secrets.h src/
```

### При синхронизации между местами:

```bash
# На основном ПК
cd ~/private-secrets
git add .
git commit -m "Update secrets"
git push

# На другом ПК
cd ~/private-secrets
git pull
./sync.sh  # Запускаете скрипт синхронизации
```

---

## 🤖 Бонус: Работа с GitHub Actions

Для автоматической сборки без раскрытия секретов используйте GitHub Secrets:

```yaml
# .github/workflows/build.yml
name: Build Project

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v3

    - name: Create secrets.h
      run: |
        cat > src/secrets.h << EOF
        #pragma once
        #define WIFI_SSID "${{ secrets.WIFI_SSID }}"
        #define WIFI_PASS "${{ secrets.WIFI_PASS }}"
        #define TELEGRAM_TOKEN "${{ secrets.TELEGRAM_TOKEN }}"
        EOF

    - name: Build with PlatformIO
      run: |
        pip install platformio
        platformio run
```

В настройках репозитория на GitHub добавляете:
- `WIFI_SSID`
- `WIFI_PASS`
- `TELEGRAM_TOKEN`

---

## ✅ Преимущества подхода

| Преимущество | Описание |
|--------------|----------|
| 🔒 **Безопасность** | Секреты не попадают в публичные репозитории |
| 🔄 **Синхронизация** | Легко переносить между рабочими местами |
| 🎯 **Гибкость** | Быстрое переключение между локациями |
| 🛠 **Простота** | Работает с любой IDE (VSCode, PlatformIO, Arduino IDE) |
| 📦 **Совместимость** | Идеально сочетается с вашей библиотекой ETL |
| 👥 **Командная работа** | Новые разработчики видят структуру, но не видят реальных данных |