#include "mynet.h"

void exit_errmesg(char *errmesg) {
    perror(errmesg);
    exit(EXIT_FAILURE);
}

/*
  パケットの種類=type のパケットをbufferに作成する
  作成されるパケットのデータを格納するためのメモリ領域は呼び出し前に
　確保しておく必要がある
*/
char *create_packet(char *buffer, u_int32_t type, const char *message) {
    switch (type) {
        case HELLO:
            snprintf(buffer, MSG_BUFSIZE, "HELO");
            break;
        case HERE:
            snprintf(buffer, MSG_BUFSIZE, "HERE");
            break;
        case JOIN:
            snprintf(buffer, MSG_BUFSIZE, "JOIN %s", message);
            break;
        case POST:
            snprintf(buffer, MSG_BUFSIZE, "POST %s", message);
            break;
        case MESSAGE:
            snprintf(buffer, MSG_BUFSIZE, "MESG %s", message);
            break;
        case QUIT:
            snprintf(buffer, MSG_BUFSIZE, "QUIT");
            break;
        default:
            /* Undefined packet type */
            return (NULL);
            break;
    }
    return (buffer);
}

/*-------------------------------------------
 * TCP用
 -------------------------------------------*/

int init_tcpserver(in_port_t myport, int backlog) {
    struct sockaddr_in my_adrs;
    int sock_listen;

    memset(&my_adrs, 0, sizeof(my_adrs));
    my_adrs.sin_family = AF_INET;
    my_adrs.sin_port = htons(myport);
    my_adrs.sin_addr.s_addr = htonl(INADDR_ANY);

    if ((sock_listen = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        exit_errmesg("socket()");
    }

    if (bind(sock_listen, (struct sockaddr *)&my_adrs, sizeof(my_adrs)) == -1) {
        exit_errmesg("bind()");
    }

    if (listen(sock_listen, backlog) == -1) {
        exit_errmesg("listen()");
    }

    return sock_listen;
}

int init_tcpclient(char *servername, in_port_t serverport) {
    struct hostent *server_host;
    struct sockaddr_in server_adrs;
    int sock;

    if ((server_host = gethostbyname(servername)) == NULL) {
        exit_errmesg("gethostbyname()");
    }

    memset(&server_adrs, 0, sizeof(server_adrs));
    server_adrs.sin_family = AF_INET;
    server_adrs.sin_port = htons(serverport);
    memcpy(&server_adrs.sin_addr, server_host->h_addr_list[0],
           server_host->h_length);

    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        exit_errmesg("socket()");
    }

    if (connect(sock, (struct sockaddr *)&server_adrs, sizeof(server_adrs)) ==
        -1) {
        exit_errmesg("connect()");
    }

    return sock;
}

int Accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
    int r;
    if ((r = accept(s, addr, addrlen)) == -1) {
        exit_errmesg("accept()");
    }
    return (r);
}

int Send(int s, void *buf, size_t len, int flags) {
    int r;
    if ((r = send(s, buf, len, flags)) == -1) {
        exit_errmesg("send()");
    }
    return (r);
}

int Recv(int s, void *buf, size_t len, int flags) {
    int r;
    if ((r = recv(s, buf, len, flags)) == -1) {
        exit_errmesg("recv()");
    }
    return (r);
}

/*-------------------------------------------
 * UDP用
 -------------------------------------------*/

int init_udpserver(in_port_t myport) {
    struct sockaddr_in my_adrs;
    int sock;

    memset(&my_adrs, 0, sizeof(my_adrs));
    my_adrs.sin_family = AF_INET;
    my_adrs.sin_port = htons(myport);
    my_adrs.sin_addr.s_addr = htonl(INADDR_ANY);

    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
        exit_errmesg("socket()");
    }

    if (bind(sock, (struct sockaddr *)&my_adrs, sizeof(my_adrs)) == -1) {
        exit_errmesg("bind()");
    }

    return sock;
}

int init_udpclient() {
    int sock;

    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
        exit_errmesg("socket()");
    }

    return sock;
}

void set_sockaddr_in(struct sockaddr_in *server_adrs, char *servername,
                     in_port_t port_number) {
    struct hostent *server_host;

    /* サーバ名をアドレス(hostent構造体)に変換する */
    if ((server_host = gethostbyname(servername)) == NULL) {
        exit_errmesg("gethostbyname()");
    }

    /* サーバの情報をsockaddr_in構造体に格納する */
    memset(server_adrs, 0, sizeof(struct sockaddr_in));
    server_adrs->sin_family = AF_INET;
    server_adrs->sin_port = htons(port_number);
    memcpy(&(server_adrs->sin_addr), server_host->h_addr_list[0],
           server_host->h_length);
}

void set_sockaddr_in_broadcast(struct sockaddr_in *server_adrs,
                               in_port_t port_number) {
    /* ブロードキャストアドレスの情報をsockaddr_in構造体に格納する */
    memset(server_adrs, 0, sizeof(struct sockaddr_in));
    server_adrs->sin_family = AF_INET;
    server_adrs->sin_port = htons(port_number);
    server_adrs->sin_addr.s_addr = htonl(INADDR_BROADCAST);
}

int Sendto(int sock, const void *s_buf, size_t strsize, int flags,
           const struct sockaddr *to, socklen_t tolen) {
    int r;
    if ((r = sendto(sock, s_buf, strsize, 0, to, tolen)) == -1) {
        exit_errmesg("sendto()");
    }

    return (r);
}

int Recvfrom(int sock, void *r_buf, size_t len, int flags,
             struct sockaddr *from, socklen_t *fromlen) {
    int r;
    if ((r = recvfrom(sock, r_buf, len, 0, from, fromlen)) == -1) {
        exit_errmesg("recvfrom()");
    }

    return (r);
}