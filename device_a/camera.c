/*
 * Larscope — Camera Capture Module
 * Initializes GStreamer pipeline for video capture, split, and upscale.
 */
#include "../shared/module.h"
#include "../shared/logger.h"
#include "../shared/config.h"
#include <gst/gst.h>

static struct {
    GstElement *pipeline;
    GstElement *src;
    GstElement *tee;
    GstElement *scale;
    GstBus     *bus;
    guint       bus_watch_id;
} g_ctx;

static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data) {
    (void)bus;
    (void)data;
    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS:
            ls_log(LS_LOG_INFO, "camera", "End of stream");
            break;
        case GST_MESSAGE_ERROR: {
            gchar *debug;
            GError *error;
            gst_message_parse_error(msg, &error, &debug);
            ls_log(LS_LOG_ERROR, "camera", "GStreamer Error: %s", error->message);
            g_free(debug);
            g_error_free(error);
            break;
        }
        default:
            break;
    }
    return TRUE;
}

static int camera_init(ls_module_t *mod) {
    (void)mod;
    const ls_config_t *cfg = ls_config_get();
    
    g_ctx.pipeline = gst_pipeline_new("camera-pipeline");
    if (!g_ctx.pipeline) {
        ls_log(LS_LOG_ERROR, "camera", "Failed to create pipeline");
        return -1;
    }
    
    /* Elements: v4l2src ! jpegdec ! videoscale ! video/x-raw,width=4K,height=4K ! tee */
    g_ctx.src = gst_element_factory_make("v4l2src", "source");
    GstElement *jpegdec = gst_element_factory_make("jpegdec", "decoder");
    g_ctx.scale = gst_element_factory_make("videoscale", "scaler");
    GstElement *filter = gst_element_factory_make("capsfilter", "filter");
    g_ctx.tee = gst_element_factory_make("tee", "cam_tee");
    
    if (!g_ctx.src || !jpegdec || !g_ctx.scale || !filter || !g_ctx.tee) {
        ls_log(LS_LOG_ERROR, "camera", "Failed to create GStreamer elements");
        return -1;
    }
    
    /* Configure source */
    g_object_set(G_OBJECT(g_ctx.src), "device", cfg->video_device, NULL);
    
    /* Configure upscaler caps to 4K */
    GstCaps *caps = gst_caps_new_simple("video/x-raw",
                                      "width", G_TYPE_INT, cfg->stream_width,
                                      "height", G_TYPE_INT, cfg->stream_height,
                                      NULL);
    g_object_set(G_OBJECT(filter), "caps", caps, NULL);
    gst_caps_unref(caps);
    
    /* Build pipeline */
    gst_bin_add_many(GST_BIN(g_ctx.pipeline), g_ctx.src, jpegdec, g_ctx.scale, filter, g_ctx.tee, NULL);
    if (!gst_element_link_many(g_ctx.src, jpegdec, g_ctx.scale, filter, g_ctx.tee, NULL)) {
        ls_log(LS_LOG_ERROR, "camera", "Failed to link camera elements");
        return -1;
    }
    
    /* Bus watch */
    g_ctx.bus = gst_pipeline_get_bus(GST_PIPELINE(g_ctx.pipeline));
    g_ctx.bus_watch_id = gst_bus_add_watch(g_ctx.bus, bus_call, NULL);
    
    ls_log(LS_LOG_INFO, "camera", "Initialized for %s (upscaling to %dx%d)", 
           cfg->video_device, cfg->stream_width, cfg->stream_height);
    return 0;
}

static int camera_start(ls_module_t *mod) {
    (void)mod;
    /* We don't start the pipeline yet! The recording and streaming modules
     * need to connect their branches to the 'tee' before we play. */
    ls_event_emit(EVT_CAMERA_READY, "camera");
    return 0;
}

static void camera_stop(ls_module_t *mod) {
    (void)mod;
    if (g_ctx.pipeline) {
        gst_element_set_state(g_ctx.pipeline, GST_STATE_NULL);
    }
}

static void camera_destroy(ls_module_t *mod) {
    (void)mod;
    if (g_ctx.pipeline) {
        gst_object_unref(g_ctx.pipeline);
        g_ctx.pipeline = NULL;
    }
    if (g_ctx.bus) {
        gst_object_unref(g_ctx.bus);
        g_source_remove(g_ctx.bus_watch_id);
    }
}

/* Expose the tee so other modules can attach to it */
GstElement* ls_camera_get_tee(void) {
    return g_ctx.tee;
}

/* Expose the pipeline so other modules can sync state */
GstElement* ls_camera_get_pipeline(void) {
    return g_ctx.pipeline;
}

ls_module_t g_mod_camera = {
    "camera", NULL, camera_init, camera_start, camera_stop, camera_destroy, NULL
};
