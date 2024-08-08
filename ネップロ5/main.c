#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>
#include "mynet.h"
#include "server.h"
#include "client.h"

//最大サイズなどを定義
#define USERNAME_SIZE 15
#define DEFAULT_PORT 50001
#define BUFFER_SIZE 512
#define MAX_RETRIES 3

//エラーハンドリング用
void error(const char *msg);

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <username> [port]\n", argv[0]);
        exit(1);
    }

    char username[USERNAME_SIZE];
    strncpy(username, argv[1], USERNAME_SIZE);
    username[USERNAME_SIZE - 1] = '\0'; 
    //3つ目のコマンドライン引数がなければデフォルトポート（50001）を採用
    int port = (argc == 3) ? atoi(argv[2]) : DEFAULT_PORT;

    int sock;
    struct sockaddr_in broadcast_addr, server_addr;
    char buffer[BUFFER_SIZE];
    int broadcast = 1;

    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        error("socket() error");
    }

    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *)&broadcast, sizeof(broadcast)) < 0) {
        error("setsockopt() error");
    }

    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    broadcast_addr.sin_port = htons(port);

    int retries = 0;
    int server_found = 0;
    socklen_t addr_len = sizeof(server_addr);

    // "HELO"パケットを送信
    while (retries < MAX_RETRIES && !server_found) {
        strcpy(buffer, "HELO");
        if (sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr)) < 0) {
            error("sendto() error");
        }

        // 受信待ちしてます
        fd_set read_fds;
        struct timeval timeout;
        FD_ZERO(&read_fds);
        FD_SET(sock, &read_fds);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int retval = select(sock + 1, &read_fds, NULL, NULL, &timeout);
        if (retval == -1) {
            error("select() error");
        } else if (retval > 0) {
            if (recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len) < 0) {
                error("recvfrom() error");
            }
            buffer[BUFFER_SIZE - 1] = '\0';
            //!HEREが帰ってきたら、server_foundに1を加算して87行目でモードチェンジ
            if (strcmp(buffer, "HERE") == 0) {
                server_found = 1;
            }
        }

        retries++;
    }

    close(sock);

    if (server_found) {
        // クライアントとして動く
        start_client(username, port);
    } else {
        // サーバーとして動作する
        start_server(port, username);
    }

    return 0;
}

void error(const char *msg) {
    perror(msg);
    exit(1);
}
