#include <stdlib.h>
#include <sys/select.h>

#include "chat.h"
#include "mynet.h"

#define NAMELENGTH 20 /* ログイン名の長さ制限 */
#define BUFLEN 500    /* 通信バッファサイズ */

/* 各クライアントのユーザ情報を格納する構造体の定義 */
typedef struct {
    int sock;
    char name[NAMELENGTH];
} client_info;

/* プライベート変数 */
static int N_client;             /* クライアントの数 */
static int Max_sd;               /* ディスクリプタ最大値 */
static char speaker[NAMELENGTH]; /* 話している人の名前 */
static client_info *Client;      /* クライアントの情報 */
static char sBuf[BUFLEN], rBuf[BUFLEN]; /* 送信用バッファ, 受信用ブッファ */

/* プライベート関数 */
static int login(int sock_listen);
static void chat_send();
static void chat_receive();
static char *chop_nl(char *s);

/* クライアント情報の保存のためのメモリ確保とログイン処理 */
void init_client(int sock_listen, int n_client) {
    N_client = n_client;

    /* クライアント情報の保存用構造体の初期化 */
    if ((Client = (client_info *)malloc(N_client * sizeof(client_info))) ==
        NULL) {
        exit_errmesg("malloc()");
    }

    /* クライアントのログイン処理 */
    Max_sd = login(sock_listen);
}

/* メッセージの送受信を繰り返す */
void chat_loop() {
    while (1) {
        /* メッセージの受取 */
        chat_receive();
        /* メッセージの送信 */
        chat_send();
    }
}

/* ログイン(ニックネームの登録、保存)の処理 */
static int login(int sock_listen) {
    int client_id, sock_accepted;
    static char prompt[] = "Input your name :";
    char loginname[NAMELENGTH];
    int strsize;

    for (client_id = 0; client_id < N_client; client_id++) {
        /* クライアントの接続を受け付ける */
        sock_accepted = accept(sock_listen, NULL, NULL);
        printf("Client[%d] connected.\n", client_id);

        /* ログインプロンプトを送信 */
        Send(sock_accepted, prompt, strlen(prompt), 0);

        /* ログイン名を受信 */
        strsize = recv(sock_accepted, loginname, NAMELENGTH - 1, 0);
        loginname[strsize] = '\0';
        chop_nl(loginname);

        /* ユーザ情報を保存 */
        Client[client_id].sock = sock_accepted;
        strncpy(Client[client_id].name, loginname, NAMELENGTH);
        printf("Client[%d]'s name is %s.\n", client_id, Client[client_id].name);
    }
    return (sock_accepted);
}

/* メッセージを受け取る処理 */
static void chat_receive() {
    fd_set mask, readfds;
    int client_id;
    int strsize;

    /* ビットマスクの準備 */
    FD_ZERO(&mask);
    for (client_id = 0; client_id < N_client; client_id++) {
        FD_SET(Client[client_id].sock, &mask);
    }

    /* 受信データの有無をチェック */
    readfds = mask;
    select(Max_sd + 1, &readfds, NULL, NULL, NULL);
    for (client_id = 0; client_id < N_client; client_id++) {
        if (FD_ISSET(Client[client_id].sock, &readfds)) {
            strsize = Recv(Client[client_id].sock, rBuf, BUFLEN - 1, 0);
            rBuf[strsize] = '\0';
            chop_nl(rBuf);
            /* 誰からのメッセージかを大域変数speakerに保存 */
            strcpy(speaker, Client[client_id].name);
        }
    }
}

/* メッセージの送信と表示 と 誰からのメッセージかを表示 */
static void chat_send() {
    int client_id;
    int len;
    memset(sBuf, '\0', BUFLEN);
    for (client_id = 0; client_id < N_client; client_id++) {
        if (strcmp(speaker, Client[client_id].name) != 0) {
            len = snprintf(sBuf, BUFLEN, "%s:%s\n", speaker, rBuf);
            Send(Client[client_id].sock, sBuf, len, 0);
        }
    }
}

/* 改行コードを取り除く */
static char *chop_nl(char *s) {
    int len;
    len = strlen(s);
    if (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
        s[len - 1] = '\0';
        if (len > 1 && s[len - 2] == '\r') {
            s[len - 2] = '\0';
        }
    }
    return s;
}