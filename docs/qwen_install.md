# Qwen Code — Установка и настройка

## Что такое Qwen Code

**Qwen Code** — это AI-ассистент для разработки от Alibaba, интегрируемый в VS Code. Помогает писать код, объяснять логику, рефакторить и находить ошибки.

---

## Требования

### Для Windows 11
- **ОС:** Windows 11 (64-bit)
- **Node.js:** версия 18.x или выше
- **VS Code:** версия 1.85 или выше
- **Git:** для работы с репозиториями

### Для macOS
- **ОС:** macOS 12 (Monterey) или выше
- **Node.js:** версия 18.x или выше
- **VS Code:** версия 1.85 или выше
- **Git:** для работы с репозиториями

---

## Пошаговая установка

### Шаг 1: Установка Node.js

#### Windows 11
1. Перейдите на [nodejs.org](https://nodejs.org/)
2. Скачайте **LTS-версию** (рекомендуется)
3. Запустите установщик и следуйте инструкциям
4. Проверьте установку:
   ```powershell
   node --version
   npm --version
   ```

#### macOS
1. Через Homebrew (рекомендуется):
   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   brew install node@18
   ```
2. Или скачайте с [nodejs.org](https://nodejs.org/)

---

### Шаг 2.1: Установка qwen code для командной строки
Install Qwen Code:

NPM(recommended) (Win11)
```powershell
npm install -g @qwen-code/qwen-code@latest
```

Homebrew(macOS, Linux)
```bash
brew install qwen-code
```

Start using Qwen Code:
```
cd your-project
qwen
```
Select Qwen OAuth (Free) authentication and follow the prompts to log in. Then let’s start with understanding your codebase.

---

### Шаг 2: Установка VS Code

1. Перейдите на [code.visualstudio.com](https://code.visualstudio.com/)
2. Скачайте версию для вашей ОС
3. Установите и запустите VS Code

---

### Шаг 3: Установка расширения Qwen Code

#### Через VS Code Marketplace
1. Откройте VS Code
2. Перейдите в **Extensions** (`Ctrl+Shift+X` или `Cmd+Shift+X`)
3. В поиске введите: **Qwen Code**
4. Найдите расширение от **Alibaba Cloud**
5. Нажмите **Install**

#### Прямая ссылка
- [Qwen Code на Marketplace](https://marketplace.visualstudio.com/items?itemName=alibaba-cloud.qwen-code)

### IntelliJ IDEA Plugin
1. Откройте IntelliJ IDEA
2. Перейдите в **Preferences** (`Ctrl+Alt+S` или `Cmd+,`)
3. Перейдите в **Plugins**
4. Нажмите **Browse repositories...**
5. В поиске введите: **Lingma Alibaba Cloud AI Coding Asssitant**
6. Найдите плагин от **Lingma**
7. Нажмите **Install**
8. Перезагрузите IntelliJ IDEA
9. После перезагрузки, плагин попросит авторизацию через аккаунт Alibaba Cloud или API-ключ (если есть). Есть нет аккаунта, его можно бесплатно зарегистрировать на [Alibaba Cloud](https://www.alibabacloud.com/).

#### Ссылки

- [Lingma Alibaba Cloud AI Coding Asssitant](https://plugins.jetbrains.com/plugin/17809-lingma--alibaba-cloud-ai-coding-assistant)
- [Lingma Quick Start](https://www.alibabacloud.com/help/en/lingma/getting-started/lingma-ide-get-started?spm=a2c63.p38356.help-menu-2804669.d_1_0.1531ebb88g6QQh)

### IntelliJ MCP Server

1. Устанавливаем qwen-code
2. Запускаем, делаем первичные настройки: авторизуемся в chat.qwen.ai
3. Идем в настройки IDEA, Tools -> MCP Server.
4. Включаем MCP Server
5. Копируем настройки с помощью Copy SSE Config
6. Открываем файл `~/.qwen/settings.json` (на маке)
7. Добавляем в корень json новый параметр `mcpServers` и в нём прописываем наш сервер.

```json
"mcpServers": {
    "idea": {
      "type": "sse",
      "url": "http://localhost:64342/sse",
      "headers": {
        "IJ_MCP_SERVER_PROJECT_PATH": null
      }
    }
  }
```

8. Запускаем qwen-code и выполняем команду `/mcp list`

[Интеграция coding-агента с MCP-сервером Idea](https://habr.com/ru/articles/989716/)

---

### Шаг 4: Настройка после установки

1. **Откройте панель Qwen Code:**
   - Нажмите `Ctrl+Shift+P` (Windows) или `Cmd+Shift+P` (macOS)
   - Введите: `Qwen Code: Open Panel`

2. **Авторизация:**
   - При первом запуске потребуется войти через аккаунт Alibaba Cloud
   - Или используйте API-ключ (если есть)

3. **Настройка языка:**
   - По умолчанию Qwen Code отвечает на языке пользователя
   - Для принудительной настройки создайте файл `.qwen/output-language.md` в домашней директории:
     ```markdown
     # Output language preference: Russian
     <!-- qwen-code:llm-output-language: Russian -->
     ```

---

## Дополнительные компоненты

### Git (для работы с репозиториями)

#### Windows 11
```powershell
winget install Git.Git
```
Или скачайте с [git-scm.com](https://git-scm.com/)

#### macOS
```bash
xcode-select --install
# или через Homebrew
brew install git
```

---

## Проверка работоспособности

1. Откройте любой проект в VS Code
2. Откройте панель Qwen Code (`Ctrl+Shift+P` → `Qwen Code: Open Panel`)
3. Задайте вопрос, например:
   ```
   Объясни, что делает этот файл?
   ```

---

## Полезные команды

| Команда | Описание |
|---------|----------|
| `Ctrl+Shift+P` → `Qwen Code: Open Panel` | Открыть панель ассистента |
| `Ctrl+Shift+P` → `Qwen Code: Clear Context` | Очистить контекст диалога |
| `Ctrl+Shift+P` → `Qwen Code: Explain Code` | Объяснить выбранный код |
| `Ctrl+Shift+P` → `Qwen Code: Generate Tests` | Создать тесты для файла |

---

## Решение проблем

### Qwen Code не появляется в списке расширений
- Проверьте версию VS Code (должна быть 1.85+)
- Обновите VS Code до последней версии

### Ошибка авторизации
- Проверьте подключение к интернету
- Попробуйте войти через веб-интерфейс: [chat.qwen.ai](https://chat.qwen.ai/)

### Ассистент не отвечает на русском
- Создайте файл `.qwen/output-language.md` (см. выше)
- Перезапустите VS Code

---

## Ссылки

- **Официальная документация:** [Qwen Code Docs](https://help.aliyun.com/zh/qwen)
- **Веб-интерфейс:** [chat.qwen.ai](https://chat.qwen.ai/)
- **GitHub:** [QwenLM/Qwen](https://github.com/QwenLM/Qwen)
- **VS Code Marketplace:** [Qwen Code Extension](https://marketplace.visualstudio.com/items?itemName=alibaba-cloud.qwen-code)
- **IntelliJ IDEA Plugin:** [Lingma Alibaba Cloud AI Coding Asssitant](https://plugins.jetbrains.com/plugin/17809-lingma--alibaba-cloud-ai-coding-assistant)
- **IntelliJ IDEA QWEN MCP Integration:** [Интеграция coding-агента с MCP-сервером Idea](https://habr.com/ru/articles/989716/)

---

*Последнее обновление: 2026-02-19*
