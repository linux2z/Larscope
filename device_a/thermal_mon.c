/*
 * Larscope — Thermal Monitor Module
 * Monitors SoC temperature.
 */
#include "../shared/module.h"
#include "../shared/logger.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define THERMAL_PATH "/sys/class/thermal/thermal_zone0/temp"

static struct {
    pthread_t thread;
    int running;
} g_ctx;

static void* thermal_monitor_thread(void *arg) {
    (void)arg;
    char buf[32];
    
    while (g_ctx.running) {
        FILE *f = fopen(THERMAL_PATH, "r");
        if (f) {
            if (fgets(buf, sizeof(buf), f)) {
                int millidegrees = atoi(buf);
                float temp = millidegrees / 1000.0f;
                
                ls_event_t evt;
                memset(&evt, 0, sizeof(evt));
                evt.type = EVT_THERMAL_UPDATE;
                strncpy(evt.source, "thermal", sizeof(evt.source)-1);
                
                ls_payload_thermal_t *payload = (ls_payload_thermal_t*)evt.payload;
                payload->soc_temp = temp;
                evt.payload_len = sizeof(ls_payload_thermal_t);
                
                ls_event_publish(&evt);
                
                if (temp > 75.0f) {
                    ls_log(LS_LOG_WARN, "thermal", "SoC temperature high: %.1f C", temp);
                    ls_event_emit(EVT_THERMAL_WARNING, "thermal");
                }
            }
            fclose(f);
        }
        sleep(5);
    }
    return NULL;
}

static int thermal_init(ls_module_t *mod) {
    (void)mod;
    g_ctx.running = 0;
    ls_log(LS_LOG_INFO, "thermal", "Initialized");
    return 0;
}

static int thermal_start(ls_module_t *mod) {
    (void)mod;
    g_ctx.running = 1;
    if (pthread_create(&g_ctx.thread, NULL, thermal_monitor_thread, NULL) != 0) {
        ls_log(LS_LOG_ERROR, "thermal", "Failed to start thread");
        return -1;
    }
    return 0;
}

static void thermal_stop(ls_module_t *mod) {
    (void)mod;
    if (g_ctx.running) {
        g_ctx.running = 0;
        pthread_join(g_ctx.thread, NULL);
    }
}

static void thermal_destroy(ls_module_t *mod) {
    (void)mod;
}

ls_module_t g_mod_thermal = {
    "thermal", NULL, thermal_init, thermal_start, thermal_stop, thermal_destroy, NULL
};
