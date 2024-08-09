#ifndef MYNET_H_
#define MYNET_H_

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MSG_BUFSIZE 512

#define HELLO 1
#define HERE 2
#define JOIN 3
#define POST 4
#define MESSAGE 5
#define QUIT 6

#define DEFAULT_PORT 50001
#define S_BUFSIZE 512 /* 送信用バッファサイズ */
#define R_BUFSIZE 512 /* 受信用バッファサイズ */
#define TIMEOUT_SEC 3

void exit_errmesg(char *errmesg);
char *create_packet(char *buffer, u_int32_t type, const char *message);

// TCP用
int init_tcpserver(in_port_t myport, int backlog);
int init_tcpclient(char *servername, in_port_t serverport);
/* Accept関数(エラー処理つき) */
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen);
/* 送信関数(エラー処理つき) */
int Send(int s, void *buf, size_t len, int flags);
/* 受信関数(エラー処理つき) */
int Recv(int s, void *buf, size_t len, int flags);

// UDP用
int init_udpserver(in_port_t myport);
int init_udpclient();
void set_sockaddr_in(struct sockaddr_in *server_adrs, char *servername,
                     in_port_t port_number);
void set_sockaddr_in_broadcast(struct sockaddr_in *server_adrs,
                               in_port_t port_number);
/* 送信関数(エラー処理つき) */
int Sendto(int sock, const void *s_buf, size_t strsize, int flags,
           const struct sockaddr *to, socklen_t tolen);
/* 受信関数(エラー処理つき) */
int Recvfrom(int sock, void *r_buf, size_t len, int flags,
             struct sockaddr *from, socklen_t *fromlen);
#endif /* MYNET_H_ */