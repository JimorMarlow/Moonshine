#pragma once
/**
 * @file web-ui.h
 * @brief Веб-интерфейс Moonshine: сервер контента и менеджер переключения серверов
 *
 * Платформа: ESP32
 *
 * Особенности:
 * - Сервер контента (moonshine_web_server) построен на базовом классе
 *   etl::webui::web_server_base_t из библиотеки ETL
 * - Настройка сети, темы и шрифта выполняется редко, поэтому для неё запускается
 *   отдельный сервер etl::webui::server_setup: в обычном режиме память под него не занята
 * - При переключении режима Wi-Fi не переинициализируется: точка доступа остаётся
 *   поднятой, меняется только HTTP-сервер
 * - Оба сервера создаются один раз, их HTTP-серверы поднимаются и гасятся по требованию
 * - Локальное имя хоста: "moonshine"
 * - Реальное время: данные получаются из settings::moonshine::state_t
 *
 * @note Для обновления макета веб-интерфейса:
 *       1. Взять актуальный файл из docs/hystory/qwen-distillation.xxx.html
 *       2. Извлечь HTML/CSS/JS код
 *       3. Заменить данные симуляции на функцию get_state()
 *       4. Обновить версию в заголовке на MS_VERSION_STRING
 */

#include <Arduino.h>

#include "settings.h"
#include "version.h"

#include <etl/etl_memory.h>
#include <etl/etl_webui.h>
#include <etl/etl_webui_base.h>
#include <etl/etl_webui_settings.h>

namespace webui
{
    /**
     * @brief Конфигурация WiFi по-умолчанию для проекта
     *
     * Используется при инициализации настроек и сбросе к заводским значениям.
     */
    etl::webui::server_config_t make_default_config();

    /**
     * @brief Конфигурация интерфейса по-умолчанию для проекта
     */
    etl::webui::ui_config_t make_default_ui_config();

    /**
     * @brief Информация об устройстве для веб-серверов
     *
     * В энергонезависимой памяти не хранится: при обновлении прошивки
     * название и описание устройства обновляются автоматически.
     */
    etl::webui::device_info_t make_device_info();

    /**
     * @brief Сервер контента: веб-интерфейс мониторинга процесса дистилляции
     *
     * Предоставляет страницу мониторинга и JSON с текущими показаниями датчиков.
     * Wi-Fi и mDNS обслуживаются базовым классом ETL.
     */
    class moonshine_web_server : public etl::webui::web_server_base_t
    {
    public:
        /// Тип функции получения состояния системы
        using state_getter_t = settings::moonshine::state_t (*)();

        moonshine_web_server();
        virtual ~moonshine_web_server() = default;

        /**
         * @brief Установить функцию получения состояния системы
         *
         * Функция общая для всех экземпляров сервера контента.
         *
         * @param getter Функция, возвращающая settings::moonshine::state_t
         */
        static void set_state_getter(state_getter_t getter);

        /**
         * @brief Погасить только HTTP-сервер, оставив Wi-Fi поднятым
         */
        void stop_http_only();

        /**
         * @brief Поднять HTTP-сервер без инициализации Wi-Fi
         */
        void start_http_only();

    protected:
        /**
         * @brief Запуск HTTP сервера и регистрация обработчиков
         */
        virtual void start_http_server() override;

        /**
         * @brief Обработчик корневой страницы
         * Отдаёт HTML страницу веб-интерфейса
         */
        virtual void handle_root();

        /**
         * @brief Обработчик API состояния
         * Отдаёт JSON с текущими показаниями датчиков
         */
        virtual void handle_api_state();

        /**
         * @brief Обработчик API статуса
         * Отдаёт JSON со статусом системы (uptime, режим работы и т.д.)
         */
        virtual void handle_api_status();

        /**
         * @brief Обработчик запроса на сервер настроек
         * Отдаёт страницу переключения, затем запрашивает сервер настроек
         */
        virtual void handle_api_settings();

        /**
         * @brief Генерация JSON из состояния системы
         * @param state Состояние системы
         * @return JSON строка
         */
        virtual String state_to_json(const settings::moonshine::state_t& state);

        static state_getter_t s_state_getter;  ///< Функция получения состояния
    };

    /**
     * @brief Сервер настроек Moonshine
     *
     * Библиотечный etl::webui::server_setup с возможностью поднять и погасить только
     * HTTP-сервер: при переключении режима Wi-Fi не переинициализируется, поэтому
     * точка доступа не пропадает из списка сетей.
     */
    class moonshine_settings_server : public etl::webui::server_setup
    {
    public:
        explicit moonshine_settings_server(const etl::webui::server_config_t& cfg = make_default_config());

        /**
         * @brief Погасить только HTTP-сервер, оставив Wi-Fi поднятым
         */
        void stop_http_only();

        /**
         * @brief Поднять HTTP-сервер без инициализации Wi-Fi
         */
        void start_http_only();
    };

    /**
     * @brief Менеджер веб-серверов Moonshine
     *
     * Держит оба сервера и переключает их HTTP-часть, не трогая Wi-Fi: точка доступа
     * поднимается один раз при старте и остаётся поднятой до перезагрузки контроллера.
     * Переключение выполняется из loop(), после того как сервер отправил ответ клиенту:
     * останавливать сервер внутри его же обработчика HTTP нельзя.
     */
    class moonshine_web_manager : public etl::webui::web_manager
    {
    public:
        explicit moonshine_web_manager(const etl::webui::device_info_t& device_info);
        virtual ~moonshine_web_manager();

        /**
         * @brief Первый запуск: поднять Wi-Fi и запустить нужный сервер
         * @param settings_mode true - сервер настроек, false - сервер контента
         * @return true при успешном запуске
         */
        bool start(bool settings_mode);

        /**
         * @brief Запросить переключение на сервер настроек
         * @note Вызывается из callback'а сервера, только выставляет флаг
         */
        void request_settings();

        /**
         * @brief Запросить переключение на сервер контента
         * @note Вызывается из callback'а сервера, только выставляет флаг
         */
        void request_content();

        /**
         * @brief Выполнить отложенное переключение серверов
         * @note Вызывать в loop() после tick()
         */
        void handle_pending_switch();

        /**
         * @brief Вывести в Serial, какой сервер сейчас работает
         *
         * В отличие от etl::webui::web_manager::trace_connection(), тип сервера берётся
         * не из режима Wi-Fi (в AP оба сервера выглядят одинаково), а из реального состояния.
         */
        void trace_active_server() const;

    protected:
        /**
         * @brief Создать сервер контента
         */
        virtual etl::shared_ptr<etl::webui::web_server_base_t> on_create_content() override;

        /**
         * @brief Создать сервер настроек
         */
        virtual etl::shared_ptr<etl::webui::web_server_base_t> on_create_settings() override;

        /**
         * @brief Создать оба сервера, если они ещё не созданы
         */
        void ensure_servers();

        /**
         * @brief Активировать сервер контента (Wi-Fi не трогается)
         */
        void activate_content();

        /**
         * @brief Активировать сервер настроек (Wi-Fi не трогается)
         */
        void activate_settings();

        etl::shared_ptr<moonshine_web_server> m_content;       ///< Сервер контента
        etl::shared_ptr<moonshine_settings_server> m_settings; ///< Сервер настроек
        bool m_settings_active = false;                        ///< Активен сервер настроек
        volatile bool m_settings_requested = false;            ///< Запрошен сервер настроек
        volatile bool m_content_requested = false;             ///< Запрошен сервер контента
    };

} // namespace webui
