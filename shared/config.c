/*
 * Larscope — Config Manager Implementation
 */
#include "config.h"
#include "logger.h"
#include <json-c/json.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static ls_config_t g_config;
static char g_config_path[256] = "/etc/larscope/config.json";

static void set_defaults(void) {
    memset(&g_config, 0, sizeof(g_config));
    
    g_config.log_level = LS_LOG_INFO;
    strcpy(g_config.log_file, "/var/log/larscope.log");
    strcpy(g_config.storage_path, "/mnt/sdcard/larscope");
    
    strcpy(g_config.video_device, "/dev/video21");
    g_config.capture_width = 1920;
    g_config.capture_height = 1080;
    g_config.capture_fps = 30;
    
    strcpy(g_config.stream_protocol, "rtsp");
    g_config.stream_port = 8554;
    strcpy(g_config.stream_codec, "h264");
    g_config.stream_width = 3840;   /* Default upscale to 4K */
    g_config.stream_height = 2160;
    g_config.stream_bitrate_kbps = 8000;
    
    strcpy(g_config.record_codec, "h265");
    g_config.record_bitrate_kbps = 10000;
    g_config.split_time_min = 30;
    
    strcpy(g_config.net_mode, "auto");
}

int ls_config_load(const char *filepath) {
    set_defaults();
    
    if (filepath) {
        strncpy(g_config_path, filepath, sizeof(g_config_path)-1);
    }
    
    FILE *f = fopen(g_config_path, "r");
    if (!f) {
        ls_log(LS_LOG_WARN, "config", "Could not open %s, using defaults", g_config_path);
        return 0; /* Not fatal, use defaults */
    }
    
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char *buf = malloc(fsize + 1);
    if (!buf) {
        fclose(f);
        return -1;
    }
    fread(buf, 1, fsize, f);
    buf[fsize] = 0;
    fclose(f);
    
    struct json_object *parsed = json_tokener_parse(buf);
    free(buf);
    
    if (!parsed) {
        ls_log(LS_LOG_ERROR, "config", "Failed to parse JSON config");
        return -1;
    }
    
    struct json_object *sys, *cam, *stream, *rec, *net, *val;
    
    if (json_object_object_get_ex(parsed, "system", &sys)) {
        if (json_object_object_get_ex(sys, "log_level", &val)) g_config.log_level = json_object_get_int(val);
        if (json_object_object_get_ex(sys, "log_file", &val)) strncpy(g_config.log_file, json_object_get_string(val), sizeof(g_config.log_file)-1);
        if (json_object_object_get_ex(sys, "storage_path", &val)) strncpy(g_config.storage_path, json_object_get_string(val), sizeof(g_config.storage_path)-1);
    }
    
    if (json_object_object_get_ex(parsed, "camera", &cam)) {
        if (json_object_object_get_ex(cam, "video_device", &val)) strncpy(g_config.video_device, json_object_get_string(val), sizeof(g_config.video_device)-1);
        if (json_object_object_get_ex(cam, "capture_width", &val)) g_config.capture_width = json_object_get_int(val);
        if (json_object_object_get_ex(cam, "capture_height", &val)) g_config.capture_height = json_object_get_int(val);
        if (json_object_object_get_ex(cam, "capture_fps", &val)) g_config.capture_fps = json_object_get_int(val);
    }
    
    if (json_object_object_get_ex(parsed, "streaming", &stream)) {
        if (json_object_object_get_ex(stream, "protocol", &val)) strncpy(g_config.stream_protocol, json_object_get_string(val), sizeof(g_config.stream_protocol)-1);
        if (json_object_object_get_ex(stream, "codec", &val)) strncpy(g_config.stream_codec, json_object_get_string(val), sizeof(g_config.stream_codec)-1);
        if (json_object_object_get_ex(stream, "width", &val)) g_config.stream_width = json_object_get_int(val);
        if (json_object_object_get_ex(stream, "height", &val)) g_config.stream_height = json_object_get_int(val);
    }
    
    if (json_object_object_get_ex(parsed, "recording", &rec)) {
        if (json_object_object_get_ex(rec, "codec", &val)) strncpy(g_config.record_codec, json_object_get_string(val), sizeof(g_config.record_codec)-1);
    }
    
    json_object_put(parsed);
    ls_log(LS_LOG_INFO, "config", "Configuration loaded successfully");
    return 0;
}

const ls_config_t* ls_config_get(void) {
    return &g_config;
}

int ls_config_save(void) {
    /* To be implemented: serialize g_config to JSON and write to file */
    return 0;
}
