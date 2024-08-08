#include <errno.h>
#include <pthread.h>

#include "mynet.h"

#define CMD_BUFSIZE 64
#define R_BUFSIZE 64
#define S_BUFSIZE 1024

void playServer(int);
void execCmd(int, char[], int, char[]);
void *echo_thread(void *arg);

/* スレッド関数の引数 */
struct myarg {
    int sock; /* acceptしたソケット */
    int id;   /* スレッドの通し番号 */
};

int main(int argc, char *argv[]) {
    int port_number;
    int sock_listen;
    int i;
    struct myarg *tharg;
    pthread_t tid;

    /* 引数のチェックと使用法の表示 */
    if (argc != 3) {
        fprintf(stderr, "Usage: %s Port_number Process_number\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    port_number = atoi(argv[1]); /* 引数の取得 */

    /* サーバの初期化 */
    sock_listen = init_tcpserver(port_number, 5);

    for (i = 0; i < atoi(argv[2]); i++) {
        /* スレッド関数の引数を用意する */
        if ((tharg = (struct myarg *)malloc(sizeof(struct myarg))) == NULL) {
            exit_errmesg("malloc()");
        }

        tharg->sock = sock_listen;
        tharg->id = i;

        /* スレッドを生成する */
        if (pthread_create(&tid, NULL, echo_thread, (void *)tharg) != 0) {
            exit_errmesg("pthread_create()");
        }
    }

    pthread_exit(NULL);
}

/* スレッドの本体 */
void *echo_thread(void *arg) {
    int sock_accepted;
    struct myarg *tharg;
    char r_buf[R_BUFSIZE], s_buf[S_BUFSIZE];
    int strsize;

    tharg = (struct myarg *)arg;
    pthread_detach(pthread_self()); /* スレッドの分離(終了を待たない) */

    while (1) {
        sock_accepted = accept(tharg->sock, NULL, NULL);
        playServer(sock_accepted);
        close(sock_accepted); /* ソケットを閉じる */
    }
    free(tharg); /* 引数用のメモリを開放する */
    return (NULL);
}

void playServer(int sock_accepted) {
    int strsize;
    char r_buf[R_BUFSIZE], s_buf[S_BUFSIZE], cmd_buf[CMD_BUFSIZE], *token,
        *endptr;

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
        // for (int i = 0; i < strlen(r_buf); i++) printf("->%i",
        // r_buf[i]); putchar('\n');

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
}

void execCmd(int sock, char buf[], int bufsize, char cmd[]) {
    FILE *fp;
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