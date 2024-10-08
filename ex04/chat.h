/*
  chat.h
*/
#ifndef CHAT_H_
#define CHAT_H_
#include "mynet.h"

/* サーバメインルーチン */
void chat_server(int port_number, int n_client);

/* クライアントメインルーチン */
void chat_client(char *servername, int port_number);

/* クライアントの初期化 */
void init_client(int sock_listen, int n_client);

/* メインループ */
void chat_loop();

/* accept関数(エラー処理つき) */
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen);

/* 送信関数(エラー処理つき) */
int Send(int s, void *buf, size_t len, int flags);

/* 受信関数(エラー処理つき) */
int Recv(int s, void *buf, size_t len, int flags);

#endif /* CHAT_H_ */