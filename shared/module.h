/*
 * Larscope — Module Framework
 * Common interface for all system modules.
 */
#ifndef LS_MODULE_H
#define LS_MODULE_H

#include "event_bus.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declaration */
struct ls_module;

/* ── Module Interface ───────────────────────────────────────── */

/**
 * Initialize the module. Called once at startup before any module starts.
 * Perform allocations, hardware detection, etc.
 * @return 0 on success, <0 on failure.
 */
typedef int (*ls_module_init_fn)(struct ls_module *mod);

/**
 * Start the module. Called after all modules have initialized.
 * Start threads, open files, begin streaming, etc.
 * @return 0 on success, <0 on failure.
 */
typedef int (*ls_module_start_fn)(struct ls_module *mod);

/**
 * Stop the module. Called during shutdown.
 * Stop threads, flush data, etc.
 */
typedef void (*ls_module_stop_fn)(struct ls_module *mod);

/**
 * Destroy the module. Free allocations, close handles.
 */
typedef void (*ls_module_destroy_fn)(struct ls_module *mod);

/**
 * Optional: Handle events subscribed via the event bus.
 * This is just a convenience signature if the module wants a central handler.
 */
typedef void (*ls_module_event_fn)(struct ls_module *mod, const ls_event_t *event);

typedef struct ls_module {
    const char           *name;
    void                 *priv;      /* Private data pointer for the module */
    
    ls_module_init_fn     init;
    ls_module_start_fn    start;
    ls_module_stop_fn     stop;
    ls_module_destroy_fn  destroy;
    ls_module_event_fn    handle_event;
} ls_module_t;

/* ── Module Manager API ─────────────────────────────────────── */

/**
 * Register a module with the system.
 */
int ls_module_register(ls_module_t *mod);

/**
 * Initialize all registered modules.
 */
int ls_module_init_all(void);

/**
 * Start all registered modules.
 */
int ls_module_start_all(void);

/**
 * Stop all registered modules (in reverse order of start).
 */
void ls_module_stop_all(void);

/**
 * Destroy all registered modules (in reverse order of init).
 */
void ls_module_destroy_all(void);


/* ── Declare System Modules ─────────────────────────────────── */
/* These will be implemented in their respective files. */

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


#ifdef __cplusplus
}
#endif

#endif /* LS_MODULE_H */
