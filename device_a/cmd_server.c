/*
 * Larscope — Command Server
 * Listens on TCP 8601 for Device B commands (JSON) and converts them to internal events.
 */
#include "../shared/module.h"
#include "../shared/logger.h"
#include <json-c/json.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

static struct {
    pthread_t thread;
    int running;
    int server_fd;
} g_ctx;

static void handle_client(int client_fd) {
    char buf[1024];
    int n = read(client_fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = 0;
        struct json_object *parsed = json_tokener_parse(buf);
        if (parsed) {
            struct json_object *cmd_obj;
            if (json_object_object_get_ex(parsed, "command", &cmd_obj)) {
                const char *cmd = json_object_get_string(cmd_obj);
                ls_log(LS_LOG_INFO, "cmd_server", "Received command: %s", cmd);
                
                if (strcmp(cmd, "start_record") == 0) {
                    ls_event_emit(EVT_RECORD_START, "cmd_server");
                } else if (strcmp(cmd, "stop_record") == 0) {
                    ls_event_emit(EVT_RECORD_STOP, "cmd_server");
                } else if (strcmp(cmd, "capture_image") == 0) {
                    ls_event_emit(EVT_CAPTURE_IMAGE, "cmd_server");
                }
            }
            json_object_put(parsed);
        }
    }
    
    /* Send ACK */
    const char *ack = "{\"status\":\"ok\"}\n";
    write(client_fd, ack, strlen(ack));
    close(client_fd);
}

static void* cmd_server_thread(void *arg) {
    (void)arg;
    struct sockaddr_in addr;
    
    g_ctx.server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(g_ctx.server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8601);
    
    if (bind(g_ctx.server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        ls_log(LS_LOG_ERROR, "cmd_server", "Failed to bind port 8601");
        return NULL;
    }
    
    listen(g_ctx.server_fd, 5);
    ls_log(LS_LOG_INFO, "cmd_server", "Listening on port 8601");
    
    while (g_ctx.running) {
        /* In a real implementation we should use select/epoll to handle shutdown cleanly */
        int client_fd = accept(g_ctx.server_fd, NULL, NULL);
        if (client_fd >= 0) {
            handle_client(client_fd);
        }
    }
    return NULL;
}

static int cmd_server_init(ls_module_t *mod) {
    (void)mod;
    memset(&g_ctx, 0, sizeof(g_ctx));
    return 0;
}

static int cmd_server_start(ls_module_t *mod) {
    (void)mod;
    g_ctx.running = 1;
    if (pthread_create(&g_ctx.thread, NULL, cmd_server_thread, NULL) != 0) return -1;
    return 0;
}

static void cmd_server_stop(ls_module_t *mod) {
    (void)mod;
    if (g_ctx.running) {
        g_ctx.running = 0;
        if (g_ctx.server_fd >= 0) close(g_ctx.server_fd);
        pthread_join(g_ctx.thread, NULL);
    }
}

static void cmd_server_destroy(ls_module_t *mod) {
    (void)mod;
}

ls_module_t g_mod_cmd_server = {
    "cmd_server", NULL, cmd_server_init, cmd_server_start, cmd_server_stop, cmd_server_destroy, NULL
};
