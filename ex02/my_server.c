#include <errno.h>

#include "mynet.h"

#define CMD_BUFSIZE 64
#define R_BUFSIZE 64
#define S_BUFSIZE 1024

void execCmd(int, char[], int, char*);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "ERROR: invalid arguments\n");
        fprintf(stderr, "Usage: ./my_server <port>\n");
    }

    int port, sock_listen, sock_accepted, strsize;
    char r_buf[R_BUFSIZE], s_buf[S_BUFSIZE], cmd_buf[CMD_BUFSIZE], *token,
        *endptr;

    errno = 0;
    port = strtol(argv[1], &endptr, 10);
    if (endptr == argv[1] || *endptr != '\0' || errno != 0) {
        exit_errmesg("strtol()");
    }

    sock_listen = init_tcpserver(port, 5);
    printf("Waiting for connections ...\n");
    sock_accepted = accept(sock_listen, NULL, NULL);
    printf("Connection established.\n");

    // パスワード確認部
    send(sock_accepted, "Enter password: ", 17, 0);
    recv(sock_accepted, r_buf, R_BUFSIZE, 0);
    printf("password: %s", r_buf);
    if (strcmp(r_buf, "sample_pass\r\n") != 0) {
        send(sock_accepted, "Error: invalid password\n", 25, 0);
        close(sock_listen);
        close(sock_accepted);
        exit(EXIT_FAILURE);
    }

    while (1) {
        // バッファの初期化
        memset(cmd_buf, 0, CMD_BUFSIZE);
        memset(r_buf, 0, R_BUFSIZE);
        memset(s_buf, 0, S_BUFSIZE);

        // '>'をクライアントに表示させる
        if (send(sock_accepted, ">", 1, 0) == -1) {
            exit_errmesg("send()");
        }

        // 文字列をクライアントから受信する
        if ((strsize = recv(sock_accepted, r_buf, R_BUFSIZE - 1, 0)) == -1) {
            exit_errmesg("recv()");
        }

        // 末尾を終端文字にする
        r_buf[strsize] = '\0';

        // '\r\n'を削除
        for (int i = 0; i < strsize; i++) {
            if (r_buf[i] == '\r' || r_buf[i] == '\n') {
                r_buf[i] = '\0';
                break;
            }
        }

        // debug
        printf("string: %s\n", r_buf);
        // printf("strsize: %i\n", strsize);
        // printf("(first)");
        // for (int i = 0; i < strlen(r_buf); i++) printf("->%i", r_buf[i]);
        // putchar('\n');

        // コマンドごとに処理
        if (strsize == 2) continue;  // 改行のみ
        token = strtok(r_buf, " ");
        if (token == NULL) continue;  // 空白のみ
        if (strcmp(token, "exit") == 0) break;
        if (strcmp(token, "list") == 0) {
            execCmd(sock_accepted, token, S_BUFSIZE, "ls ~/work");
        } else if (strcmp(token, "type") == 0) {
            token = strtok(NULL, " ");
            if (token == NULL) {
                send(sock_accepted, "Usage: type <filename>\n", 24, 0);
            } else {
                snprintf(cmd_buf, CMD_BUFSIZE - 1, "cat ~/work/%s", token);
                execCmd(sock_accepted, s_buf, S_BUFSIZE, cmd_buf);

                // debug
                printf("token: %s\n", token);
                printf("cmd: %s\n", cmd_buf);
            }
        } else {
            snprintf(s_buf, S_BUFSIZE - 1, "Error: invalid command \"%s\"\n",
                     token);
            send(sock_accepted, s_buf, strlen(s_buf), 0);
        }
    }

    close(sock_listen);
    close(sock_accepted);

    exit(EXIT_SUCCESS);
}

void execCmd(int sock, char buf[], int bufsize, char cmd[]) {
    FILE* fp;
    if ((fp = popen(cmd, "r")) == NULL) {
        perror("popen()");
        return;
    }

    while (fgets(buf, bufsize, fp) != NULL) {
        if (send(sock, buf, strlen(buf), 0) == -1) {
            perror("send() in execCmd");
            return;
        }
    }
    if (buf[strlen(buf) - 1] != '\n') send(sock, "\n", 1, 0);

    int status = pclose(fp);

    // debug
    printf("exit status: %d\n", status);
}