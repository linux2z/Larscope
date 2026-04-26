/*
 * Larscope Device B — Main Entry Point
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <gst/gst.h>

extern int stream_client_init(const char *rtsp_url);
extern void stream_client_stop(void);
extern int cmd_client_init(const char *host, int port);
extern int cmd_client_send(const char *cmd);

static int g_running = 1;

static void handle_signal(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        printf("Received signal %d, shutting down...\n", sig);
        g_running = 0;
    }
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    gst_init(&argc, &argv);

    printf("Larscope Device B starting up\n");

    /* Default IP of Device A in this setup */
    const char *target_ip = "192.168.137.59";
    char rtsp_url[256];
    snprintf(rtsp_url, sizeof(rtsp_url), "rtsp://%s:8554/live", target_ip);

    if (stream_client_init(rtsp_url) < 0) {
        printf("Failed to init stream client\n");
        return -1;
    }

    if (cmd_client_init(target_ip, 8601) < 0) {
        printf("Warning: Command client connection failed\n");
    }

    printf("System running. Press Ctrl+C to exit.\n");

    /* For demonstration, send a capture command after 5 seconds */
    int ticks = 0;
    while (g_running) {
        sleep(1);
        ticks++;
        if (ticks == 5) {
            printf("Sending capture_image command...\n");
            cmd_client_send("capture_image");
        }
    }

    stream_client_stop();
    printf("Device B shutdown complete.\n");
    return 0;
}
