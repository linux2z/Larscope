/*
 * Larscope — Recording Module
 * Taps into camera tee to record H.264/H.265 via Rockchip MPP.
 */
#include "../shared/module.h"
#include "../shared/logger.h"
#include "../shared/config.h"
#include "camera.h"
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <sys/time.h>
#include <stdio.h>

static struct {
    GstElement *queue;
    GstElement *encoder;
    GstElement *muxer;
    GstElement *sink;
    GstElement *snap_queue;
    GstElement *snap_conv;
    GstElement *snap_enc;
    GstElement *snap_sink;
    GstPad     *tee_src_pad;
    GstPad     *snap_tee_pad;
    int         recording;
    int         capture_requested;
} g_ctx;

static GstFlowReturn on_new_sample(GstAppSink *appsink, gpointer data) {
    (void)data;
    GstSample *sample = gst_app_sink_pull_sample(appsink);
    if (g_ctx.capture_requested && sample) {
        g_ctx.capture_requested = 0;
        GstBuffer *buffer = gst_sample_get_buffer(sample);
        GstMapInfo map;
        if (gst_buffer_map(buffer, &map, GST_MAP_READ)) {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            char filename[512];
            snprintf(filename, sizeof(filename), "%s/snap_%ld.jpg", ls_config_get()->storage_path, tv.tv_sec);
            
            FILE *f = fopen(filename, "wb");
            if (f) {
                fwrite(map.data, 1, map.size, f);
                fclose(f);
                ls_log(LS_LOG_INFO, "recording", "Lascope: Saved still image to %s", filename);
            }
            gst_buffer_unmap(buffer, &map);
        }
    }
    if (sample) gst_sample_unref(sample);
    return GST_FLOW_OK;
}

static void handle_event(const ls_event_t *event, void *user_data) {
    (void)user_data;
    if (event->type == EVT_RECORD_START && !g_ctx.recording) {
        /* Future: Start actual file recording dynamically */
        ls_log(LS_LOG_INFO, "recording", "Recording started");
        g_ctx.recording = 1;
    } else if (event->type == EVT_RECORD_STOP && g_ctx.recording) {
        ls_log(LS_LOG_INFO, "recording", "Recording stopped");
        g_ctx.recording = 0;
    } else if (event->type == EVT_CAPTURE_IMAGE) {
        g_ctx.capture_requested = 1;
        ls_log(LS_LOG_INFO, "recording", "Still image capture requested");
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
    
    g_ctx.encoder = gst_element_factory_make("x264enc", "rec_enc");
    
    g_ctx.muxer = gst_element_factory_make("mp4mux", "rec_mux");
    g_ctx.sink = gst_element_factory_make("filesink", "rec_sink");
    
    g_ctx.snap_queue = gst_element_factory_make("queue", "snap_queue");
    g_ctx.snap_conv = gst_element_factory_make("videoconvert", "snap_conv");
    g_ctx.snap_enc = gst_element_factory_make("jpegenc", "snap_enc");
    g_ctx.snap_sink = gst_element_factory_make("appsink", "snap_sink");

    if (!g_ctx.queue || !conv || !capsfilter || !g_ctx.encoder || !g_ctx.muxer || !g_ctx.sink || !g_ctx.snap_sink) {
        ls_log(LS_LOG_ERROR, "recording", "Failed to create recording elements");
        return -1;
    }

    /* Configure */
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/record.mp4", cfg->storage_path);
    g_object_set(G_OBJECT(g_ctx.sink), "location", filepath, NULL);
    
    /* Target bitrate in kbps */
    g_object_set(G_OBJECT(g_ctx.encoder), "bitrate", cfg->record_bitrate_kbps, NULL);
    
    /* Configure Appsink */
    g_object_set(G_OBJECT(g_ctx.snap_sink), "max-buffers", 1, "drop", TRUE, "emit-signals", TRUE, NULL);
    g_signal_connect(g_ctx.snap_sink, "new-sample", G_CALLBACK(on_new_sample), NULL);

    gst_bin_add_many(GST_BIN(pipeline), g_ctx.queue, conv, capsfilter, g_ctx.encoder, g_ctx.muxer, g_ctx.sink, 
                     g_ctx.snap_queue, g_ctx.snap_conv, g_ctx.snap_enc, g_ctx.snap_sink, NULL);
    
    if (!gst_element_link_many(g_ctx.queue, conv, capsfilter, g_ctx.encoder, g_ctx.muxer, g_ctx.sink, NULL)) {
        ls_log(LS_LOG_ERROR, "recording", "Failed to link recording elements");
        return -1;
    }
    
    if (!gst_element_link_many(g_ctx.snap_queue, g_ctx.snap_conv, g_ctx.snap_enc, g_ctx.snap_sink, NULL)) {
        ls_log(LS_LOG_ERROR, "recording", "Failed to link snapshot elements");
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
    
    /* Link tee to snap_queue */
    g_ctx.snap_tee_pad = gst_element_get_request_pad(tee, "src_%u");
    GstPad *snap_sink_pad = gst_element_get_static_pad(g_ctx.snap_queue, "sink");
    gst_pad_link(g_ctx.snap_tee_pad, snap_sink_pad);
    gst_object_unref(snap_sink_pad);

    ls_event_subscribe(EVT_RECORD_START, handle_event, NULL);
    ls_event_subscribe(EVT_RECORD_STOP, handle_event, NULL);
    ls_event_subscribe(EVT_CAPTURE_IMAGE, handle_event, NULL);

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
