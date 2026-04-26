/*
 * Larscope — Event Bus
 * Central publish-subscribe event dispatcher.
 * All inter-module communication goes through this bus.
 */
#ifndef LS_EVENT_BUS_H
#define LS_EVENT_BUS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Event Types ────────────────────────────────────────────── */
typedef enum {
    /* System */
    EVT_NONE = 0,
    EVT_SYSTEM_INIT,
    EVT_SYSTEM_SHUTDOWN,
    EVT_SYSTEM_ERROR,

    /* Buttons */
    EVT_BUTTON_PRESS,
    EVT_BUTTON_RELEASE,

    /* LEDs */
    EVT_LED_STATUS_SET,
    EVT_LED_ILLUM_SET,

    /* Camera / Video */
    EVT_CAMERA_READY,
    EVT_CAPTURE_IMAGE,
    EVT_ZOOM_IN,
    EVT_ZOOM_OUT,

    /* Recording */
    EVT_RECORD_START,
    EVT_RECORD_STOP,
    EVT_CAPTURE_IMAGE,
    EVT_ZOOM_IN,
    EVT_ZOOM_OUT,
    EVT_ILLUM_ZONE1_CYCLE,
    EVT_ILLUM_ZONE2_CYCLE,
    EVT_BATTERY_UPDATE,
    EVT_SYSTEM_STATE_REQ,

    /* Streaming */
    EVT_STREAM_START,
    EVT_STREAM_STOP,
    EVT_STREAM_CLIENT_CONNECT,
    EVT_STREAM_CLIENT_DISCONNECT,

    /* Network */
    EVT_NETWORK_UP,
    EVT_NETWORK_DOWN,
    EVT_NETWORK_CHANGE,

    /* Thermal */
    EVT_THERMAL_UPDATE,
    EVT_THERMAL_WARNING,
    EVT_THERMAL_CRITICAL,

    /* Config */
    EVT_CONFIG_CHANGED,

    /* Commands (from Device B) */
    EVT_CMD_RECEIVED,

    /* Storage */
    EVT_STORAGE_LOW,
    EVT_FILE_SAVED,

    EVT_TYPE_COUNT
} ls_event_type_t;

/* ── Event Payload ──────────────────────────────────────────── */
#define LS_EVT_PAYLOAD_MAX 256

typedef struct {
    ls_event_type_t type;
    uint64_t        timestamp_ms;     /* milliseconds since epoch */
    char            source[32];       /* module name that emitted */
    uint8_t         payload[LS_EVT_PAYLOAD_MAX];
    size_t          payload_len;
} ls_event_t;

/* ── Common Payloads ────────────────────────────────────────── */
typedef struct {
    char button[16];   /* "middle", "right", "left", "upper", "lower" */
    char gpio[16];     /* "GPIO1_D6", etc. */
} ls_payload_button_t;

typedef struct {
    char  led[16];     /* "power", "battery", "charging", "wifi", "recording" */
    int   state;       /* 0=off, 1=on, 2=blink, 3=pulse */
    int   color;       /* 0=default, 1=red, 2=green */
} ls_payload_led_t;

typedef struct {
    int   group;       /* 1 or 2 */
    int   brightness;  /* 0-100 */
} ls_payload_illum_t;

typedef struct {
    float soc_temp;
    float max_temp;
} ls_payload_thermal_t;

/* ── Callback Signature ─────────────────────────────────────── */
typedef void (*ls_event_cb_t)(const ls_event_t *event, void *user_data);

/* ── API ────────────────────────────────────────────────────── */

/**
 * Initialize the event bus. Call once at startup.
 * Returns 0 on success, -1 on error.
 */
int ls_event_bus_init(void);

/**
 * Shutdown and free all resources.
 */
void ls_event_bus_destroy(void);

/**
 * Subscribe to an event type.
 * @param type     Event type to listen for.
 * @param cb       Callback function.
 * @param user_data Opaque pointer passed to callback.
 * Returns subscription ID (>= 0), or -1 on error.
 */
int ls_event_subscribe(ls_event_type_t type, ls_event_cb_t cb, void *user_data);

/**
 * Unsubscribe by subscription ID.
 */
void ls_event_unsubscribe(int sub_id);

/**
 * Publish an event. All subscribers for the event type are called
 * synchronously in the caller's thread.
 * @param event  The event to publish. Timestamp is auto-filled if 0.
 */
void ls_event_publish(ls_event_t *event);

/**
 * Helper: create and publish an event with no payload.
 */
void ls_event_emit(ls_event_type_t type, const char *source);

/**
 * Helper: get current timestamp in milliseconds.
 */
uint64_t ls_time_ms(void);

#ifdef __cplusplus
}
#endif

#endif /* LS_EVENT_BUS_H */
