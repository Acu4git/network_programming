#include "server.h"

#include "mynet.h"

#define BACKLOG 5

void run_server(const char* username, int port) {
    int udp_sock, tcp_sock;
    struct sockaddr_in from_adrs;
    socklen_t from_len = sizeof(from_adrs);

    udp_sock = init_udpserver(port);
}