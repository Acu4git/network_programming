#include <sys/wait.h>

#include "mynet.h"

#define CMD_BUFSIZE 64
#define R_BUFSIZE 64
#define S_BUFSIZE 1024
#define PRCS_LIMIT 10

void playServer(int);
void execCmd(int, char[], int, char*);

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s Port_number Process_number\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    pid_t child;
    int n_process = 0;
    int port_number, sock_listen, sock_accepted;

    port_number = atoi(argv[1]);

    sock_listen = init_tcpserver(port_number, 5);
    printf("Waiting for connections ...\n");

    for (int i = 0; i < atoi(argv[2]); i++) {
        if ((child = fork()) == 0) {
            /* Child process */
            sock_accepted = accept(sock_listen, NULL, NULL);
            printf("Connection established.\n");
            close(sock_listen);

            // クライアントとのやり取りを行う
            playServer(sock_accepted);

            close(sock_accepted);
            exit(EXIT_SUCCESS);
        } else if (child > 0) {
            /* parent's process */
            n_process++;
            printf("Client is accepted.[%d]\n", child);
            close(sock_accepted);
        } else {
            /* fork()に失敗 */
            close(sock_listen);
            exit_errmesg("fork()");
        }

        /* ゾンビプロセスの回収 */
        if (n_process == PRCS_LIMIT) {
            child = wait(NULL); /* 制限数を超えたら 空きが出るまでブロック */
            n_process--;
        }

        while ((child = waitpid(-1, NULL, WNOHANG)) > 0) {
            n_process--;
        }
    }

    exit(EXIT_SUCCESS);
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