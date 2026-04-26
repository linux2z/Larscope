/*
 * Larscope — Main Entry Point (Device A)
 */
#include "../shared/event_bus.h"
#include "../shared/module.h"
#include "../shared/config.h"
#include "../shared/logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <gst/gst.h>

static int g_running = 1;

static void handle_signal(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        ls_log(LS_LOG_WARN, "main", "Received signal %d, initiating shutdown...", sig);
        g_running = 0;
    }
}

extern ls_module_t g_mod_gpio_input;
extern ls_module_t g_mod_status_led;
extern ls_module_t g_mod_illum_led;
extern ls_module_t g_mod_thermal;
extern ls_module_t g_mod_camera;
extern ls_module_t g_mod_recording;
extern ls_module_t g_mod_streaming;
extern ls_module_t g_mod_network;
extern ls_module_t g_mod_storage;
extern ls_module_t g_mod_cmd_server;

static void setup_modules(void) {
    ls_module_register(&g_mod_gpio_input);
    ls_module_register(&g_mod_status_led);
    ls_module_register(&g_mod_illum_led);
    ls_module_register(&g_mod_thermal);
    ls_module_register(&g_mod_camera);
    ls_module_register(&g_mod_recording);
    ls_module_register(&g_mod_streaming);
    ls_module_register(&g_mod_network);
    ls_module_register(&g_mod_storage);
    ls_module_register(&g_mod_cmd_server);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    /* 1. Core Framework Init */
    ls_config_load("config.json");
    const ls_config_t *cfg = ls_config_get();
    
    ls_logger_init(cfg->log_level, NULL); /* For now, just stdout */
    ls_log(LS_LOG_INFO, "main", "Larscope Device A starting up");

    gst_init(&argc, &argv);
    
    if (ls_event_bus_init() < 0) {
        ls_log(LS_LOG_FATAL, "main", "Failed to initialize event bus");
    }

    /* 2. Module Registration & Init */
    setup_modules();
    
    if (ls_module_init_all() < 0) {
        ls_log(LS_LOG_FATAL, "main", "Module initialization failed");
    }

    /* 3. Start System */
    ls_event_emit(EVT_SYSTEM_INIT, "main");
    
    if (ls_module_start_all() < 0) {
        ls_log(LS_LOG_FATAL, "main", "Module start failed");
    }

    ls_log(LS_LOG_INFO, "main", "System fully started. Press Ctrl+C to stop.");

    /* 4. Main Event Loop */
    while (g_running) {
        /* In a real implementation, we might run a glib main loop here 
         * or sleep if modules manage their own threads. */
        sleep(1);
    }

    /* 5. Shutdown */
    ls_log(LS_LOG_INFO, "main", "Shutting down modules...");
    ls_event_emit(EVT_SYSTEM_SHUTDOWN, "main");
    
    ls_module_stop_all();
    ls_module_destroy_all();
    ls_event_bus_destroy();
    ls_logger_destroy();

    return 0;
}
