/*
 * Larscope — Network Manager
 * Monitors eth0/wlan0 state and publishes events.
 */
#include "../shared/module.h"
#include "../shared/logger.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

static struct {
    pthread_t thread;
    int running;
    int eth_up;
    int wlan_up;
} g_ctx;

static int check_iface_up(const char *iface) {
    char path[256];
    char buf[32];
    snprintf(path, sizeof(path), "/sys/class/net/%s/operstate", iface);
    
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    
    int up = 0;
    if (fgets(buf, sizeof(buf), f)) {
        if (strncmp(buf, "up", 2) == 0) up = 1;
    }
    fclose(f);
    return up;
}

static void* network_monitor_thread(void *arg) {
    (void)arg;
    
    while (g_ctx.running) {
        int eth_now = check_iface_up("eth0");
        int wlan_now = check_iface_up("wlan0");
        
        if (eth_now != g_ctx.eth_up || wlan_now != g_ctx.wlan_up) {
            ls_log(LS_LOG_INFO, "network", "State changed: eth0=%s, wlan0=%s", 
                   eth_now ? "UP" : "DOWN", wlan_now ? "UP" : "DOWN");
            
            g_ctx.eth_up = eth_now;
            g_ctx.wlan_up = wlan_now;
            
            ls_event_emit((eth_now || wlan_now) ? EVT_NETWORK_UP : EVT_NETWORK_DOWN, "network");
            ls_event_emit(EVT_NETWORK_CHANGE, "network");
        }
        sleep(2);
    }
    return NULL;
}

static int network_init(ls_module_t *mod) {
    (void)mod;
    memset(&g_ctx, 0, sizeof(g_ctx));
    ls_log(LS_LOG_INFO, "network", "Initialized");
    return 0;
}

static int network_start(ls_module_t *mod) {
    (void)mod;
    g_ctx.running = 1;
    if (pthread_create(&g_ctx.thread, NULL, network_monitor_thread, NULL) != 0) {
        return -1;
    }
    return 0;
}

static void network_stop(ls_module_t *mod) {
    (void)mod;
    if (g_ctx.running) {
        g_ctx.running = 0;
        pthread_join(g_ctx.thread, NULL);
    }
}

static void network_destroy(ls_module_t *mod) {
    (void)mod;
}

ls_module_t g_mod_network = {
    "network", NULL, network_init, network_start, network_stop, network_destroy, NULL
};
