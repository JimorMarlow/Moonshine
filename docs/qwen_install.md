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

---

*Последнее обновление: 2026-02-19*
