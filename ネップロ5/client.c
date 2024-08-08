#include "client.h"

#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mynet.h"

#define MESSAGE_SIZE 488
#define USERNAME_SIZE 15
#define BUFFER_SIZE 512
#define MAX_RETRIES 3

void *receive_messages(void *socket_desc);

void start_client(const char *username, int port) {
    int udp_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int retries = 0;

    udp_socket = init_udpclient();
    send_udp_broadcast(udp_socket, "HELO", port);

    while (retries < MAX_RETRIES) {
        recv_udp_message(udp_socket, buffer, BUFFER_SIZE, &server_addr);
        if (strcmp(buffer, "HERE") == 0) {
            printf("Server found at %s\n", inet_ntoa(server_addr.sin_addr));
            break;
        }
        sleep(1);
        retries++;
    }

    if (retries == MAX_RETRIES) {
        printf("No server found, exiting\n");
        close(udp_socket);
        exit(1);
    }

    int tcp_socket = init_tcpclient(inet_ntoa(server_addr.sin_addr), port);
    printf("Connected to server at %s:%d\n", inet_ntoa(server_addr.sin_addr),
           port);

    // Send JOIN message
    snprintf(buffer, BUFFER_SIZE, "%s", username);
    if (send(tcp_socket, buffer, strlen(buffer), 0) < 0) {
        perror("Send JOIN failed");
        close(tcp_socket);
        exit(1);
    }
    printf("JOIN message sent: %s\n", buffer);

    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, receive_messages,
                       (void *)&tcp_socket) < 0) {
        perror("Could not create receive thread");
        close(tcp_socket);
        exit(1);
    }
    printf("Receive thread started\n");

    char message[MESSAGE_SIZE];
    while (1) {
        fgets(message, MESSAGE_SIZE, stdin);
        message[strcspn(message, "\n")] = '\0';
        if (strcmp(message, "QUIT") == 0) {
            snprintf(buffer, BUFFER_SIZE, "QUIT");
            if (send(tcp_socket, buffer, strlen(buffer), 0) < 0) {
                perror("Send QUIT failed");
            }
            break;
        } else {
            snprintf(buffer, BUFFER_SIZE, "POST %s", message);
            if (send(tcp_socket, buffer, strlen(buffer), 0) < 0) {
                perror("Send POST failed");
            }
        }
    }

    close(tcp_socket);
    pthread_cancel(recv_thread);
    pthread_join(recv_thread, NULL);
}

void *receive_messages(void *socket_desc) {
    int sock = *(int *)socket_desc;
    char buffer[BUFFER_SIZE];
    int len;

    while ((len = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[len] = '\0';
        printf("%s\n", buffer);
    }

    if (len < 0) {
        perror("Receive failed");
    }

    return NULL;
}
