/*
 * Larscope Device B — Command Client
 * Sends JSON commands over TCP to Device A.
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static struct {
    char target_ip[64];
    int port;
} g_ctx;

int cmd_client_init(const char *host, int port) {
    strncpy(g_ctx.target_ip, host, sizeof(g_ctx.target_ip) - 1);
    g_ctx.port = port;
    return 0;
}

int cmd_client_send(const char *cmd) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(g_ctx.port);
    
    if (inet_pton(AF_INET, g_ctx.target_ip, &server_addr.sin_addr) <= 0) {
        close(sock);
        return -1;
    }
    
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(sock);
        return -1;
    }
    
    char payload[256];
    snprintf(payload, sizeof(payload), "{\"command\":\"%s\"}\n", cmd);
    
    if (send(sock, payload, strlen(payload), 0) < 0) {
        close(sock);
        return -1;
    }
    
    /* Wait for ACK */
    char buf[256];
    int n = read(sock, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = 0;
        printf("Command Client RX: %s\n", buf);
    }
    
    close(sock);
    return 0;
}
