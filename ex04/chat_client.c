#include <stdlib.h>
#include <sys/select.h>

#include "chat.h"
#include "mynet.h"

#define S_BUFSIZE 100 /* 送信用バッファサイズ */
#define R_BUFSIZE 100 /* 受信用バッファサイズ */

/* クライアントの本体部分、主にサーバーとの接続や受送信を行う */
void chat_client(char* servername, int port_number) {
    int sock;
    int strsize;
    char s_buf[S_BUFSIZE], r_buf[R_BUFSIZE];
    fd_set mask, readfds;

    /* サーバに接続する */
    sock = init_tcpclient(servername, port_number);
    printf("connected.\n");

    /* ビットマスクの準備 */
    FD_ZERO(&mask);
    FD_SET(0, &mask);
    FD_SET(sock, &mask);

    while (1) {
        /* 受信データの有無をチェック */
        readfds = mask;
        select(sock + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(0, &readfds)) {
            /* キーボードから文字列を入力する */
            fgets(s_buf, S_BUFSIZE, stdin);
            strsize = strlen(s_buf);
            Send(sock, s_buf, strsize, 0);
        }

        if (FD_ISSET(sock, &readfds)) {
            /* サーバから文字列を受信する */
            strsize = Recv(sock, r_buf, R_BUFSIZE - 1, 0);
            r_buf[strsize] = '\0';
            printf("%s", r_buf);
            fflush(stdout); /* バッファの内容を強制的に出力 */
        }
    }
}