#include "mynet.h"

void exit_errmesg(char *errmesg) {
    perror(errmesg);
    exit(EXIT_FAILURE);
}

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
    memcpy(&server_adrs.sin_addr, server_host->h_addr_list[0], server_host->h_length);

    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        exit_errmesg("socket()");
    }

    if (connect(sock, (struct sockaddr *)&server_adrs, sizeof(server_adrs)) == -1) {
        exit_errmesg("connect()");
    }

    return sock;
}

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

void send_udp_broadcast(int sock, char *message, int port) {
    struct sockaddr_in broadcast_addr;
    int broadcast_permission = 1;

    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(port);
    broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *)&broadcast_permission, sizeof(broadcast_permission)) < 0) {
        exit_errmesg("setsockopt()");
    }

    if (sendto(sock, message, strlen(message), 0, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr)) < 0) {
        exit_errmesg("sendto()");
    }
}

void recv_udp_message(int sock, char *buffer, int buffer_size, struct sockaddr_in *sender_addr) {
    socklen_t addr_len = sizeof(struct sockaddr_in);

    if (recvfrom(sock, buffer, buffer_size, 0, (struct sockaddr *)sender_addr, &addr_len) < 0) {
        exit_errmesg("recvfrom()");
    }
}
