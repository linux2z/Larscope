/*
 * Larscope — Storage Manager
 * Exposes recorded files for Device B sync via TCP 8600.
 */
#include "../shared/module.h"
#include "../shared/logger.h"
#include "../shared/config.h"
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>

static struct {
    pthread_t thread;
    int running;
    int server_fd;
} g_ctx;

static void handle_client(int client_fd) {
    const ls_config_t *cfg = ls_config_get();
    
    char buf[1024];
    int n = read(client_fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = 0;
        
        /* Very basic protocol for MVP:
         * "LIST" -> returns JSON array of files
         * "GET filename" -> returns file binary
         */
        if (strncmp(buf, "LIST", 4) == 0) {
            char resp[4096] = "{\"files\":[";
            DIR *d = opendir(cfg->storage_path);
            if (d) {
                struct dirent *dir;
                int first = 1;
                while ((dir = readdir(d)) != NULL) {
                    if (strstr(dir->d_name, ".mp4") || strstr(dir->d_name, ".jpg")) {
                        char path[1024];
                        snprintf(path, sizeof(path), "%s/%s", cfg->storage_path, dir->d_name);
                        struct stat st;
                        if (stat(path, &st) == 0) {
                            if (!first) strcat(resp, ",");
                            char entry[256];
                            snprintf(entry, sizeof(entry), "{\"name\":\"%s\",\"size\":%ld}", dir->d_name, st.st_size);
                            strcat(resp, entry);
                            first = 0;
                        }
                    }
                }
                closedir(d);
            }
            strcat(resp, "]}");
            write(client_fd, resp, strlen(resp));
            
        } else if (strncmp(buf, "GET ", 4) == 0) {
            /* Not fully implemented in MVP yet, just returning a stub */
            const char *err = "Not Implemented\n";
            write(client_fd, err, strlen(err));
        }
    }
    close(client_fd);
}

static void* storage_server_thread(void *arg) {
    (void)arg;
    struct sockaddr_in addr;
    
    g_ctx.server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(g_ctx.server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8600);
    
    if (bind(g_ctx.server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        ls_log(LS_LOG_ERROR, "storage", "Failed to bind port 8600");
        return NULL;
    }
    
    listen(g_ctx.server_fd, 5);
    ls_log(LS_LOG_INFO, "storage", "Sync server listening on port 8600");
    
    while (g_ctx.running) {
        int client_fd = accept(g_ctx.server_fd, NULL, NULL);
        if (client_fd >= 0) {
            handle_client(client_fd);
        }
    }
    return NULL;
}

static int storage_init(ls_module_t *mod) {
    (void)mod;
    memset(&g_ctx, 0, sizeof(g_ctx));
    return 0;
}

static int storage_start(ls_module_t *mod) {
    (void)mod;
    g_ctx.running = 1;
    if (pthread_create(&g_ctx.thread, NULL, storage_server_thread, NULL) != 0) return -1;
    return 0;
}

static void storage_stop(ls_module_t *mod) {
    (void)mod;
    if (g_ctx.running) {
        g_ctx.running = 0;
        if (g_ctx.server_fd >= 0) close(g_ctx.server_fd);
        pthread_join(g_ctx.thread, NULL);
    }
}

static void storage_destroy(ls_module_t *mod) {
    (void)mod;
}

ls_module_t g_mod_storage = {
    "storage", NULL, storage_init, storage_start, storage_stop, storage_destroy, NULL
};
