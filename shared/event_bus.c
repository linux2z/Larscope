/*
 * Larscope — Event Bus Implementation
 * Thread-safe publish-subscribe event dispatcher.
 */
#include "event_bus.h"
#include "logger.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define MAX_SUBSCRIBERS 128

typedef struct {
    int              id;
    ls_event_type_t  type;
    ls_event_cb_t    cb;
    void            *user_data;
    int              active;
} subscriber_t;

static subscriber_t  g_subs[MAX_SUBSCRIBERS];
static int           g_next_id = 1;
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static int           g_initialized = 0;

/* ── Timestamp Helper ───────────────────────────────────────── */

uint64_t ls_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

/* ── Init / Destroy ─────────────────────────────────────────── */

int ls_event_bus_init(void) {
    pthread_mutex_lock(&g_mutex);
    memset(g_subs, 0, sizeof(g_subs));
    g_next_id = 1;
    g_initialized = 1;
    pthread_mutex_unlock(&g_mutex);
    ls_log(LS_LOG_INFO, "event_bus", "Event bus initialized");
    return 0;
}

void ls_event_bus_destroy(void) {
    pthread_mutex_lock(&g_mutex);
    memset(g_subs, 0, sizeof(g_subs));
    g_initialized = 0;
    pthread_mutex_unlock(&g_mutex);
    ls_log(LS_LOG_INFO, "event_bus", "Event bus destroyed");
}

/* ── Subscribe / Unsubscribe ────────────────────────────────── */

int ls_event_subscribe(ls_event_type_t type, ls_event_cb_t cb, void *user_data) {
    if (!cb) return -1;

    pthread_mutex_lock(&g_mutex);
    int slot = -1;
    for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
        if (!g_subs[i].active) {
            slot = i;
            break;
        }
    }
    if (slot < 0) {
        pthread_mutex_unlock(&g_mutex);
        ls_log(LS_LOG_ERROR, "event_bus", "No subscriber slots available");
        return -1;
    }

    g_subs[slot].id = g_next_id++;
    g_subs[slot].type = type;
    g_subs[slot].cb = cb;
    g_subs[slot].user_data = user_data;
    g_subs[slot].active = 1;

    int id = g_subs[slot].id;
    pthread_mutex_unlock(&g_mutex);

    ls_log(LS_LOG_DEBUG, "event_bus", "Subscribed id=%d to event type %d", id, type);
    return id;
}

void ls_event_unsubscribe(int sub_id) {
    pthread_mutex_lock(&g_mutex);
    for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
        if (g_subs[i].active && g_subs[i].id == sub_id) {
            g_subs[i].active = 0;
            break;
        }
    }
    pthread_mutex_unlock(&g_mutex);
}

/* ── Publish ────────────────────────────────────────────────── */

void ls_event_publish(ls_event_t *event) {
    if (!event || !g_initialized) return;

    if (event->timestamp_ms == 0) {
        event->timestamp_ms = ls_time_ms();
    }

    /*
     * Collect matching subscribers under lock,
     * then call them outside the lock to avoid deadlocks.
     */
    ls_event_cb_t cbs[MAX_SUBSCRIBERS];
    void         *uds[MAX_SUBSCRIBERS];
    int           count = 0;

    pthread_mutex_lock(&g_mutex);
    for (int i = 0; i < MAX_SUBSCRIBERS && count < MAX_SUBSCRIBERS; i++) {
        if (g_subs[i].active && g_subs[i].type == event->type) {
            cbs[count] = g_subs[i].cb;
            uds[count] = g_subs[i].user_data;
            count++;
        }
    }
    pthread_mutex_unlock(&g_mutex);

    for (int i = 0; i < count; i++) {
        cbs[i](event, uds[i]);
    }
}

void ls_event_emit(ls_event_type_t type, const char *source) {
    ls_event_t evt;
    memset(&evt, 0, sizeof(evt));
    evt.type = type;
    if (source) {
        strncpy(evt.source, source, sizeof(evt.source) - 1);
    }
    ls_event_publish(&evt);
}
