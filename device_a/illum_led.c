/*
 * Larscope — Illumination LED Module
 * Controls TLC59108 I2C LED drivers.
 */
#include "../shared/module.h"
#include "../shared/logger.h"
#include <gpiod.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define I2C_BUS "/dev/i2c-2" /* Default, configurable later */
#define TLC_ADDR_1 0x40      /* Example addresses */
#define TLC_ADDR_2 0x41

/* TLC59108 Registers */
#define REG_MODE1      0x00
#define REG_MODE2      0x01
#define REG_PWM0       0x02
#define REG_LEDOUT0    0x0C

static struct {
    int i2c_fd;
    struct gpiod_chip *chip;
    struct gpiod_line *en_g1;
    struct gpiod_line *en_g2;
    int cur_brightness;
    int state; /* 0=OFF, 1=50%, 2=75% */
} g_ctx;

static int i2c_write_reg(int addr, uint8_t reg, uint8_t val) {
    if (g_ctx.i2c_fd < 0) return -1;
    if (ioctl(g_ctx.i2c_fd, I2C_SLAVE, addr) < 0) return -1;
    
    uint8_t buf[2] = { reg, val };
    if (write(g_ctx.i2c_fd, buf, 2) != 2) return -1;
    return 0;
}

static void set_brightness(int percent) {
    g_ctx.cur_brightness = percent;
    uint8_t pwm_val = (percent * 255) / 100;
    
    /* Set PWM for all 8 channels on both chips */
    for (int i = 0; i < 8; i++) {
        i2c_write_reg(TLC_ADDR_1, REG_PWM0 + i, pwm_val);
        i2c_write_reg(TLC_ADDR_2, REG_PWM0 + i, pwm_val);
    }
    
    ls_log(LS_LOG_DEBUG, "illum_led", "Brightness set to %d%%", percent);
}

static void cycle_brightness(void) {
    g_ctx.state = (g_ctx.state + 1) % 3;
    if (g_ctx.state == 0) set_brightness(0);
    else if (g_ctx.state == 1) set_brightness(50);
    else if (g_ctx.state == 2) set_brightness(75);
}

static void handle_event(const ls_event_t *event, void *user_data) {
    (void)user_data;
    if (event->type == EVT_BUTTON_PRESS) {
        const ls_payload_button_t *payload = (const ls_payload_button_t*)event->payload;
        if (strcmp(payload->button, "right") == 0 || strcmp(payload->button, "left") == 0) {
            /* Button controls LED cycle */
            cycle_brightness();
        }
    }
}

static int illum_led_init(ls_module_t *mod) {
    (void)mod;
    memset(&g_ctx, 0, sizeof(g_ctx));
    
    g_ctx.i2c_fd = open(I2C_BUS, O_RDWR);
    if (g_ctx.i2c_fd < 0) {
        ls_log(LS_LOG_WARN, "illum_led", "Could not open %s. Illumination disabled.", I2C_BUS);
    } else {
        /* Init TLC59108 */
        i2c_write_reg(TLC_ADDR_1, REG_MODE1, 0x00); /* Normal mode */
        i2c_write_reg(TLC_ADDR_1, REG_LEDOUT0, 0xAA); /* PWM control */
        i2c_write_reg(TLC_ADDR_1, REG_LEDOUT0+1, 0xAA);
        
        i2c_write_reg(TLC_ADDR_2, REG_MODE1, 0x00);
        i2c_write_reg(TLC_ADDR_2, REG_LEDOUT0, 0xAA);
        i2c_write_reg(TLC_ADDR_2, REG_LEDOUT0+1, 0xAA);
    }
    
    /* Setup enable GPIOs */
    g_ctx.chip = gpiod_chip_open_by_name("gpiochip0");
    if (g_ctx.chip) {
        g_ctx.en_g1 = gpiod_chip_get_line(g_ctx.chip, 21); /* GPIO0_C5 */
        g_ctx.en_g2 = gpiod_chip_get_line(g_ctx.chip, 20); /* GPIO0_C4 */
        
        if (g_ctx.en_g1) gpiod_line_request_output(g_ctx.en_g1, "larscope", 1);
        if (g_ctx.en_g2) gpiod_line_request_output(g_ctx.en_g2, "larscope", 1);
    }
    
    ls_event_subscribe(EVT_BUTTON_PRESS, handle_event, NULL);
    ls_log(LS_LOG_INFO, "illum_led", "Initialized");
    return 0;
}

static int illum_led_start(ls_module_t *mod) {
    (void)mod;
    g_ctx.state = 2; /* Default 75% on startup */
    set_brightness(75);
    return 0;
}

static void illum_led_stop(ls_module_t *mod) {
    (void)mod;
    set_brightness(0);
}

static void illum_led_destroy(ls_module_t *mod) {
    (void)mod;
    if (g_ctx.i2c_fd >= 0) close(g_ctx.i2c_fd);
    if (g_ctx.chip) {
        if (g_ctx.en_g1) gpiod_line_release(g_ctx.en_g1);
        if (g_ctx.en_g2) gpiod_line_release(g_ctx.en_g2);
        gpiod_chip_close(g_ctx.chip);
    }
}

ls_module_t g_mod_illum_led = {
    "illum_led", NULL, illum_led_init, illum_led_start, illum_led_stop, illum_led_destroy, NULL
};
