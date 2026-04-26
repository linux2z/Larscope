/*
 * Larscope Device B — Stream Client
 * Connects to Device A RTSP server and displays video.
 */
#include <gst/gst.h>
#include <stdio.h>

static struct {
    GstElement *pipeline;
    GstBus *bus;
    guint bus_watch_id;
} g_ctx;

static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data) {
    (void)bus;
    (void)data;
    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS:
            printf("Stream Client: End of stream\n");
            break;
        case GST_MESSAGE_ERROR: {
            gchar *debug;
            GError *error;
            gst_message_parse_error(msg, &error, &debug);
            printf("Stream Client Error: %s\n", error->message);
            g_free(debug);
            g_error_free(error);
            break;
        }
        default:
            break;
    }
    return TRUE;
}

int stream_client_init(const char *rtsp_url) {
    g_ctx.pipeline = gst_element_factory_make("playbin", "playbin");
    if (!g_ctx.pipeline) {
        printf("Failed to create playbin element\n");
        return -1;
    }

    g_object_set(G_OBJECT(g_ctx.pipeline), "uri", rtsp_url, NULL);
    
    /* Optimize for low latency */
    g_object_set(G_OBJECT(g_ctx.pipeline), "buffer-size", 2000000, NULL);
    g_object_set(G_OBJECT(g_ctx.pipeline), "latency", 50, NULL);

    g_ctx.bus = gst_pipeline_get_bus(GST_PIPELINE(g_ctx.pipeline));
    g_ctx.bus_watch_id = gst_bus_add_watch(g_ctx.bus, bus_call, NULL);

    gst_element_set_state(g_ctx.pipeline, GST_STATE_PLAYING);
    printf("Stream Client connecting to %s\n", rtsp_url);
    return 0;
}

void stream_client_stop(void) {
    if (g_ctx.pipeline) {
        gst_element_set_state(g_ctx.pipeline, GST_STATE_NULL);
        gst_object_unref(g_ctx.pipeline);
        g_ctx.pipeline = NULL;
    }
    if (g_ctx.bus) {
        gst_object_unref(g_ctx.bus);
        g_source_remove(g_ctx.bus_watch_id);
    }
}
