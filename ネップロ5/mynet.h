#ifndef MYNET_H
#define MYNET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFFER_SIZE 512

void exit_errmesg(char *errmesg);
int init_tcpserver(in_port_t myport, int backlog);
int init_tcpclient(char *servername, in_port_t serverport);
int init_udpserver(in_port_t myport);
int init_udpclient();
void send_udp_broadcast(int sock, char *message, int port);
void recv_udp_message(int sock, char *buffer, int buffer_size, struct sockaddr_in *sender_addr);

#endif // MYNET_H
