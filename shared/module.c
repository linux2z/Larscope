/*
 * Larscope — Module Framework Implementation
 */
#include "module.h"
#include "logger.h"
#include <string.h>

#define MAX_MODULES 32

static ls_module_t *g_modules[MAX_MODULES];
static int          g_module_count = 0;

int ls_module_register(ls_module_t *mod) {
    if (!mod || !mod->name) return -1;
    if (g_module_count >= MAX_MODULES) {
        ls_log(LS_LOG_ERROR, "module_mgr", "Too many modules registered (max %d)", MAX_MODULES);
        return -1;
    }
    
    /* Check for duplicates */
    for (int i = 0; i < g_module_count; i++) {
        if (strcmp(g_modules[i]->name, mod->name) == 0) {
            ls_log(LS_LOG_ERROR, "module_mgr", "Module '%s' already registered", mod->name);
            return -1;
        }
    }
    
    g_modules[g_module_count++] = mod;
    ls_log(LS_LOG_DEBUG, "module_mgr", "Registered module '%s'", mod->name);
    return 0;
}

int ls_module_init_all(void) {
    int errors = 0;
    for (int i = 0; i < g_module_count; i++) {
        ls_module_t *mod = g_modules[i];
        if (mod->init) {
            ls_log(LS_LOG_DEBUG, "module_mgr", "Initializing module '%s'", mod->name);
            if (mod->init(mod) < 0) {
                ls_log(LS_LOG_ERROR, "module_mgr", "Failed to initialize module '%s'", mod->name);
                errors++;
            }
        }
    }
    return errors == 0 ? 0 : -1;
}

int ls_module_start_all(void) {
    int errors = 0;
    for (int i = 0; i < g_module_count; i++) {
        ls_module_t *mod = g_modules[i];
        if (mod->start) {
            ls_log(LS_LOG_DEBUG, "module_mgr", "Starting module '%s'", mod->name);
            if (mod->start(mod) < 0) {
                ls_log(LS_LOG_ERROR, "module_mgr", "Failed to start module '%s'", mod->name);
                errors++;
            }
        }
    }
    return errors == 0 ? 0 : -1;
}

void ls_module_stop_all(void) {
    /* Stop in reverse order */
    for (int i = g_module_count - 1; i >= 0; i--) {
        ls_module_t *mod = g_modules[i];
        if (mod->stop) {
            ls_log(LS_LOG_DEBUG, "module_mgr", "Stopping module '%s'", mod->name);
            mod->stop(mod);
        }
    }
}

void ls_module_destroy_all(void) {
    /* Destroy in reverse order */
    for (int i = g_module_count - 1; i >= 0; i--) {
        ls_module_t *mod = g_modules[i];
        if (mod->destroy) {
            ls_log(LS_LOG_DEBUG, "module_mgr", "Destroying module '%s'", mod->name);
            mod->destroy(mod);
        }
    }
}
