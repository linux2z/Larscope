/*
 * Larscope — Status LED Module
 * Controls indicator LEDs based on system state.
 */
#include "../shared/module.h"
#include "../shared/logger.h"
#include <gpiod.h>
#include <string.h>

typedef struct {
    int line_offset;
    const char *name;
    struct gpiod_line *line;
} led_def_t;

static struct {
    struct gpiod_chip *chip;
    led_def_t leds[5];
    int sub_id;
} g_ctx;

static void set_led(int idx, int state) {
    if (g_ctx.leds[idx].line) {
        gpiod_line_set_value(g_ctx.leds[idx].line, state);
    }
}

static void handle_event(const ls_event_t *event, void *user_data) {
    (void)user_data;
    if (event->type == EVT_LED_STATUS_SET) {
        const ls_payload_led_t *payload = (const ls_payload_led_t*)event->payload;
        for (int i = 0; i < 5; i++) {
            if (strcmp(g_ctx.leds[i].name, payload->led) == 0) {
                set_led(i, payload->state);
                break;
            }
        }
    }
    else if (event->type == EVT_RECORD_START) {
        set_led(4, 1); /* Recording LED ON */
    }
    else if (event->type == EVT_RECORD_STOP) {
        set_led(4, 0); /* Recording LED OFF */
    }
    else if (event->type == EVT_NETWORK_UP) {
        set_led(3, 1); /* WiFi LED ON */
    }
    else if (event->type == EVT_NETWORK_DOWN) {
        set_led(3, 0); /* WiFi LED OFF */
    }
}

static int status_led_init(ls_module_t *mod) {
    (void)mod;
    memset(&g_ctx, 0, sizeof(g_ctx));
    
    g_ctx.leds[0] = (led_def_t){4, "power", NULL};
    g_ctx.leds[1] = (led_def_t){3, "battery", NULL};
    g_ctx.leds[2] = (led_def_t){2, "charging", NULL};
    g_ctx.leds[3] = (led_def_t){1, "wifi", NULL};
    g_ctx.leds[4] = (led_def_t){0, "recording", NULL};
    
    g_ctx.chip = gpiod_chip_open_by_name("gpiochip3");
    if (!g_ctx.chip) {
        ls_log(LS_LOG_WARN, "status_led", "gpiochip3 not found. Status LEDs disabled.");
        return 0;
    }
    
    for (int i = 0; i < 5; i++) {
        g_ctx.leds[i].line = gpiod_chip_get_line(g_ctx.chip, g_ctx.leds[i].line_offset);
        if (g_ctx.leds[i].line) {
            gpiod_line_request_output(g_ctx.leds[i].line, "larscope", 0);
        }
    }
    
    g_ctx.sub_id = ls_event_subscribe(EVT_LED_STATUS_SET, handle_event, NULL);
    ls_event_subscribe(EVT_RECORD_START, handle_event, NULL);
    ls_event_subscribe(EVT_RECORD_STOP, handle_event, NULL);
    ls_event_subscribe(EVT_NETWORK_UP, handle_event, NULL);
    ls_event_subscribe(EVT_NETWORK_DOWN, handle_event, NULL);
    
    ls_log(LS_LOG_INFO, "status_led", "Initialized");
    return 0;
}

static int status_led_start(ls_module_t *mod) {
    (void)mod;
    set_led(0, 1); /* Power LED ON */
    return 0;
}

static void status_led_stop(ls_module_t *mod) {
    (void)mod;
    set_led(0, 0); /* Power LED OFF */
}

static void status_led_destroy(ls_module_t *mod) {
    (void)mod;
    if (g_ctx.chip) {
        for (int i = 0; i < 5; i++) {
            if (g_ctx.leds[i].line) gpiod_line_release(g_ctx.leds[i].line);
        }
        gpiod_chip_close(g_ctx.chip);
    }
}

ls_module_t g_mod_status_led = {
    "status_led", NULL, status_led_init, status_led_start, status_led_stop, status_led_destroy, NULL
};
