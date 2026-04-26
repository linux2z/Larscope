/*
 * Larscope — Config Manager
 * Loads JSON configuration file.
 */
#ifndef LS_CONFIG_H
#define LS_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    /* System */
    int   log_level;         /* 0=DEBUG, 1=INFO, etc. */
    char  log_file[256];
    char  storage_path[256]; /* e.g., "/mnt/sdcard/larscope" */

    /* Camera */
    char  video_device[64];  /* e.g., "/dev/video21" */
    int   capture_width;
    int   capture_height;
    int   capture_fps;
    
    /* Streaming */
    char  stream_protocol[16]; /* "rtsp" or "tcp" */
    int   stream_port;
    char  stream_codec[16];    /* "h264" or "h265" */
    int   stream_width;
    int   stream_height;
    int   stream_bitrate_kbps;

    /* Recording */
    char  record_codec[16];
    int   record_bitrate_kbps;
    int   split_time_min;      /* Split recording every N mins */

    /* Network */
    char  net_mode[16];        /* "auto", "ethernet", "wifi" */
    
} ls_config_t;

/**
 * Load configuration from file. If file doesn't exist, loads defaults.
 * @return 0 on success, <0 on error.
 */
int ls_config_load(const char *filepath);

/**
 * Get pointer to the global configuration struct.
 * Treat as read-only.
 */
const ls_config_t* ls_config_get(void);

/**
 * Save current configuration back to file.
 */
int ls_config_save(void);

#ifdef __cplusplus
}
#endif

#endif /* LS_CONFIG_H */
