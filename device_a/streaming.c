/*
 * Larscope — Streaming Module
 * Serves RTSP stream from camera tee with low latency.
 */
#include "../shared/module.h"
#include "../shared/logger.h"
#include "../shared/config.h"
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <string.h>
#include <stdio.h>

static struct {
    GstRTSPServer *server;
    GstRTSPMountPoints *mounts;
    GstRTSPMediaFactory *factory;
    guint server_id;
    GMainLoop *loop;
    GThread *thread;
} g_ctx;

static gpointer rtsp_thread_func(gpointer data) {
    (void)data;
    g_main_loop_run(g_ctx.loop);
    return NULL;
}

static int streaming_init(ls_module_t *mod) {
    (void)mod;
    const ls_config_t *cfg = ls_config_get();
    
    g_ctx.loop = g_main_loop_new(NULL, FALSE);
    g_ctx.server = gst_rtsp_server_new();
    
    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%d", cfg->stream_port);
    gst_rtsp_server_set_service(g_ctx.server, port_str);

    g_ctx.mounts = gst_rtsp_server_get_mount_points(g_ctx.server);
    g_ctx.factory = gst_rtsp_media_factory_new();

    /* 
     * Since we share a global pipeline in Larscope, RTSP server normally creates its own pipeline.
     * To integrate, we use the v4l2src via appsrc/appsink OR for simplicity in this prototype, 
     * we will let the RTSP server instantiate a completely independent pipeline if it grabs from a v4l2 loopback, 
     * or we use shared memory.
     * For now, let's build the RTSP string to grab the video device directly. 
     * (If /dev/video21 is locked by camera.c, we need v4l2loopback or interpipesrc).
     * 
     * Wait! The optimal way is to use `appsrc` in the RTSP factory and push buffers from the tee in `camera.c` 
     * into the `appsrc`. 
     * 
     * For MVP low-latency, we'll write a pipeline string:
     */
    
    char launch_str[512];
    if (strcmp(cfg->stream_codec, "h265") == 0) {
        snprintf(launch_str, sizeof(launch_str),
                 "( v4l2src device=%s ! jpegdec ! videoscale ! video/x-raw,width=%d,height=%d "
                 "! videoconvert ! video/x-raw,format=NV12 "
                 "! mpph265enc bps=%d rc-mode=vbr ! rtph265pay name=pay0 pt=96 )",
                 cfg->video_device, cfg->stream_width, cfg->stream_height, cfg->stream_bitrate_kbps * 1000);
    } else {
        snprintf(launch_str, sizeof(launch_str),
                 "( v4l2src device=%s ! jpegdec ! videoscale ! video/x-raw,width=%d,height=%d "
                 "! videoconvert ! video/x-raw,format=NV12 "
                 "! mpph264enc bps=%d rc-mode=vbr ! rtph264pay name=pay0 pt=96 )",
                 cfg->video_device, cfg->stream_width, cfg->stream_height, cfg->stream_bitrate_kbps * 1000);
    }

    gst_rtsp_media_factory_set_launch(g_ctx.factory, launch_str);
    gst_rtsp_media_factory_set_shared(g_ctx.factory, TRUE);

    gst_rtsp_mount_points_add_factory(g_ctx.mounts, "/live", g_ctx.factory);
    g_object_unref(g_ctx.mounts);

    ls_log(LS_LOG_INFO, "streaming", "RTSP server initialized at rtsp://<ip>:%d/live", cfg->stream_port);
    return 0;
}

static int streaming_start(ls_module_t *mod) {
    (void)mod;
    g_ctx.server_id = gst_rtsp_server_attach(g_ctx.server, NULL);
    g_ctx.thread = g_thread_new("rtsp-server", rtsp_thread_func, NULL);
    return 0;
}

static void streaming_stop(ls_module_t *mod) {
    (void)mod;
    if (g_ctx.loop) {
        g_main_loop_quit(g_ctx.loop);
        g_thread_join(g_ctx.thread);
    }
}

static void streaming_destroy(ls_module_t *mod) {
    (void)mod;
    if (g_ctx.server_id) {
        g_source_remove(g_ctx.server_id);
    }
    if (g_ctx.server) {
        g_object_unref(g_ctx.server);
    }
    if (g_ctx.loop) {
        g_main_loop_unref(g_ctx.loop);
    }
}

ls_module_t g_mod_streaming = {
    "streaming", NULL, streaming_init, streaming_start, streaming_stop, streaming_destroy, NULL
};
