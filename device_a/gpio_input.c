/*
 * Larscope — GPIO Input Module
 * Monitors buttons and publishes events.
 */
#include "../shared/module.h"
#include "../shared/logger.h"
#include <gpiod.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    int line_offset;
    const char *name;
    const char *gpio_name;
    struct gpiod_line *line;
    int last_val;
    uint64_t last_edge_time;
} btn_def_t;

static struct {
    pthread_t thread;
    int running;
    struct gpiod_chip *chip;
    
    btn_def_t buttons[5];
} g_ctx;

static void* gpio_monitor_thread(void *arg) {
    (void)arg;
    struct gpiod_line_bulk bulk;
    gpiod_line_bulk_init(&bulk);
    
    for (int i = 0; i < 5; i++) {
        if (g_ctx.buttons[i].line) {
            gpiod_line_bulk_add(&bulk, g_ctx.buttons[i].line);
        }
    }
    
    while (g_ctx.running) {
        struct timespec ts = {0, 100000000}; /* 100ms */
        int ret = gpiod_line_event_wait_bulk(&bulk, &ts, NULL);
        if (ret > 0) {
            for (int i = 0; i < 5; i++) {
                struct gpiod_line *line = g_ctx.buttons[i].line;
                if (!line) continue;
                
                struct gpiod_line_event event;
                if (gpiod_line_event_read(line, &event) == 0) {
                    uint64_t now = ls_time_ms();
                    if (now - g_ctx.buttons[i].last_edge_time > 30) { /* 30ms debounce */
                        g_ctx.buttons[i].last_edge_time = now;
                        
                        ls_event_t evt;
                        memset(&evt, 0, sizeof(evt));
                        if (event.event_type == GPIOD_LINE_EVENT_RISING_EDGE) {
                            if (strcmp(g_ctx.buttons[i].name, "upper") == 0) evt.type = EVT_ZOOM_IN;
                            else if (strcmp(g_ctx.buttons[i].name, "lower") == 0) evt.type = EVT_ZOOM_OUT;
                            else if (strcmp(g_ctx.buttons[i].name, "middle") == 0) evt.type = EVT_CAPTURE_IMAGE;
                            else if (strcmp(g_ctx.buttons[i].name, "right") == 0) evt.type = EVT_ILLUM_ZONE1_CYCLE;
                            else if (strcmp(g_ctx.buttons[i].name, "left") == 0) evt.type = EVT_ILLUM_ZONE2_CYCLE;
                            else evt.type = EVT_BUTTON_PRESS;
                        } else {
                            evt.type = EVT_BUTTON_RELEASE;
                        }
                        
                        strncpy(evt.source, "gpio_input", sizeof(evt.source)-1);
                        
                        ls_payload_button_t *payload = (ls_payload_button_t*)evt.payload;
                        strncpy(payload->button, g_ctx.buttons[i].name, sizeof(payload->button)-1);
                        strncpy(payload->gpio, g_ctx.buttons[i].gpio_name, sizeof(payload->gpio)-1);
                        evt.payload_len = sizeof(ls_payload_button_t);
                        
                        ls_event_publish(&evt);
                        ls_log(LS_LOG_DEBUG, "gpio_input", "Button %s %s", g_ctx.buttons[i].name, 
                               evt.type == EVT_BUTTON_PRESS ? "pressed" : "released");
                    }
                }
            }
        }
    }
    return NULL;
}

static int gpio_init(ls_module_t *mod) {
    (void)mod;
    memset(&g_ctx, 0, sizeof(g_ctx));
    
    g_ctx.buttons[0] = (btn_def_t){30, "middle", "GPIO1_D6", NULL, 0, 0};
    g_ctx.buttons[1] = (btn_def_t){11, "right", "GPIO1_B3", NULL, 0, 0};
    g_ctx.buttons[2] = (btn_def_t){10, "left", "GPIO1_B2", NULL, 0, 0};
    g_ctx.buttons[3] = (btn_def_t){9, "upper", "GPIO1_B1", NULL, 0, 0};
    g_ctx.buttons[4] = (btn_def_t){8, "lower", "GPIO1_B0", NULL, 0, 0};
    
    g_ctx.chip = gpiod_chip_open_by_name("gpiochip1");
    if (!g_ctx.chip) {
        ls_log(LS_LOG_WARN, "gpio_input", "gpiochip1 not found. Buttons disabled.");
        return 0; /* Return 0 to not fail the whole system */
    }
    
    for (int i = 0; i < 5; i++) {
        g_ctx.buttons[i].line = gpiod_chip_get_line(g_ctx.chip, g_ctx.buttons[i].line_offset);
        if (g_ctx.buttons[i].line) {
            gpiod_line_request_both_edges_events(g_ctx.buttons[i].line, "larscope");
        } else {
            ls_log(LS_LOG_ERROR, "gpio_input", "Failed to get line %d", g_ctx.buttons[i].line_offset);
        }
    }
    
    ls_log(LS_LOG_INFO, "gpio_input", "Initialized");
    return 0;
}

static int gpio_start(ls_module_t *mod) {
    (void)mod;
    if (!g_ctx.chip) return 0;
    
    g_ctx.running = 1;
    if (pthread_create(&g_ctx.thread, NULL, gpio_monitor_thread, NULL) != 0) {
        ls_log(LS_LOG_ERROR, "gpio_input", "Failed to start thread");
        return -1;
    }
    return 0;
}

static void gpio_stop(ls_module_t *mod) {
    (void)mod;
    if (g_ctx.running) {
        g_ctx.running = 0;
        pthread_join(g_ctx.thread, NULL);
    }
}

static void gpio_destroy(ls_module_t *mod) {
    (void)mod;
    if (g_ctx.chip) {
        for (int i = 0; i < 5; i++) {
            if (g_ctx.buttons[i].line) gpiod_line_release(g_ctx.buttons[i].line);
        }
        gpiod_chip_close(g_ctx.chip);
    }
}

ls_module_t g_mod_gpio_input = {
    "gpio_input", NULL, gpio_init, gpio_start, gpio_stop, gpio_destroy, NULL
};
