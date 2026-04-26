/*
 * Larscope — Recording Module
 * Taps into camera tee to record H.264/H.265 via Rockchip MPP.
 */
#include "../shared/module.h"
#include "../shared/logger.h"
#include "../shared/config.h"
#include "camera.h"
#include <gst/gst.h>
#include <string.h>

static struct {
    GstElement *queue;
    GstElement *encoder;
    GstElement *muxer;
    GstElement *sink;
    GstPad     *tee_src_pad;
    int         recording;
} g_ctx;

static void handle_event(const ls_event_t *event, void *user_data) {
    (void)user_data;
    if (event->type == EVT_RECORD_START && !g_ctx.recording) {
        /* Future: Start actual file recording dynamically */
        ls_log(LS_LOG_INFO, "recording", "Recording started");
        g_ctx.recording = 1;
    } else if (event->type == EVT_RECORD_STOP && g_ctx.recording) {
        ls_log(LS_LOG_INFO, "recording", "Recording stopped");
        g_ctx.recording = 0;
    }
}

static int recording_init(ls_module_t *mod) {
    (void)mod;
    const ls_config_t *cfg = ls_config_get();
    
    GstElement *pipeline = ls_camera_get_pipeline();
    if (!pipeline) {
        ls_log(LS_LOG_ERROR, "recording", "No camera pipeline found");
        return -1;
    }

    g_ctx.queue = gst_element_factory_make("queue", "rec_queue");
    GstElement *conv = gst_element_factory_make("videoconvert", "rec_conv");
    GstElement *capsfilter = gst_element_factory_make("capsfilter", "rec_caps");
    
    GstCaps *caps = gst_caps_from_string("video/x-raw,format=NV12");
    g_object_set(G_OBJECT(capsfilter), "caps", caps, NULL);
    gst_caps_unref(caps);
    
    /* Use Rockchip HW encoder */
    if (strcmp(cfg->record_codec, "h265") == 0) {
        g_ctx.encoder = gst_element_factory_make("mpph265enc", "rec_enc");
    } else {
        g_ctx.encoder = gst_element_factory_make("mpph264enc", "rec_enc");
    }
    
    g_ctx.muxer = gst_element_factory_make("mp4mux", "rec_mux");
    g_ctx.sink = gst_element_factory_make("filesink", "rec_sink");

    if (!g_ctx.queue || !conv || !capsfilter || !g_ctx.encoder || !g_ctx.muxer || !g_ctx.sink) {
        ls_log(LS_LOG_ERROR, "recording", "Failed to create recording elements");
        return -1;
    }

    /* Configure */
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/record.mp4", cfg->storage_path);
    g_object_set(G_OBJECT(g_ctx.sink), "location", filepath, NULL);
    
    /* Target bitrate in bps */
    g_object_set(G_OBJECT(g_ctx.encoder), "bps", cfg->record_bitrate_kbps * 1000, NULL);

    gst_bin_add_many(GST_BIN(pipeline), g_ctx.queue, conv, capsfilter, g_ctx.encoder, g_ctx.muxer, g_ctx.sink, NULL);
    if (!gst_element_link_many(g_ctx.queue, conv, capsfilter, g_ctx.encoder, g_ctx.muxer, g_ctx.sink, NULL)) {
        ls_log(LS_LOG_ERROR, "recording", "Failed to link recording elements");
        return -1;
    }

    /* Link tee to queue */
    GstElement *tee = ls_camera_get_tee();
    g_ctx.tee_src_pad = gst_element_get_request_pad(tee, "src_%u");
    GstPad *queue_sink_pad = gst_element_get_static_pad(g_ctx.queue, "sink");
    if (gst_pad_link(g_ctx.tee_src_pad, queue_sink_pad) != GST_PAD_LINK_OK) {
        ls_log(LS_LOG_ERROR, "recording", "Failed to link tee to recording queue");
        return -1;
    }
    gst_object_unref(queue_sink_pad);

    ls_event_subscribe(EVT_RECORD_START, handle_event, NULL);
    ls_event_subscribe(EVT_RECORD_STOP, handle_event, NULL);

    ls_log(LS_LOG_INFO, "recording", "Initialized (%s to %s)", cfg->record_codec, filepath);
    return 0;
}

static int recording_start(ls_module_t *mod) {
    (void)mod;
    return 0;
}

static void recording_stop(ls_module_t *mod) {
    (void)mod;
}

static void recording_destroy(ls_module_t *mod) {
    (void)mod;
    if (g_ctx.tee_src_pad) {
        GstElement *tee = ls_camera_get_tee();
        gst_element_release_request_pad(tee, g_ctx.tee_src_pad);
        gst_object_unref(g_ctx.tee_src_pad);
    }
}

ls_module_t g_mod_recording = {
    "recording", NULL, recording_init, recording_start, recording_stop, recording_destroy, NULL
};
