#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>
#include "mynet.h"
#include "server.h"

#define MESSAGE_SIZE 488
#define USERNAME_SIZE 15
#define BACKLOG 5
#define BUFFER_SIZE 512

typedef struct {
    int socket;
    char username[USERNAME_SIZE];
} client_info;

client_info *clients[FD_SETSIZE];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *client_data);
void *udp_server(void *arg);

void start_server(int port, const char *username) {
    int tcp_socket, udp_socket;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    pthread_t udp_thread;

    signal(SIGPIPE, SIG_IGN);

    udp_socket = init_udpserver(port);
    if (pthread_create(&udp_thread, NULL, udp_server, (void *)&udp_socket) < 0) {
        perror("Could not create UDP server thread");
        exit(1);
    }

    tcp_socket = init_tcpserver(port, BACKLOG);
    printf("Server started on port %d\n", port);

    while (1) {
        int new_socket = accept(tcp_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (new_socket < 0) {
            perror("accept");
            continue;
        }

        client_info *new_client = malloc(sizeof(client_info));
        new_client->socket = new_socket;
        int len = recv(new_socket, new_client->username, USERNAME_SIZE - 1, 0);
        if (len <= 0) {
            perror("recv");
            close(new_socket);
            free(new_client);
            continue;
        }
        new_client->username[len] = '\0';

        // Remove "JOIN " prefix from username if present
        if (strncmp(new_client->username, "JOIN ", 5) == 0) {
            memmove(new_client->username, new_client->username + 5, strlen(new_client->username) - 4);
        }

        printf("User %s connected\n", new_client->username);

        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < FD_SETSIZE; i++) {
            if (clients[i] == NULL) {
                clients[i] = new_client;
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_client, (void *)new_client) < 0) {
            perror("could not create thread");
            close(new_socket);
            free(new_client);
            continue;
        }
        pthread_detach(client_thread);
    }

    close(tcp_socket);
}

void *udp_server(void *arg) {
    int sock = *(int *)arg;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    while (1) {
        recv_udp_message(sock, buffer, BUFFER_SIZE, &client_addr);
        if (strcmp(buffer, "HELO") == 0) {
            sendto(sock, "HERE", 4, 0, (struct sockaddr *)&client_addr, addr_len);
        }
    }

    return NULL;
}

void *handle_client(void *client_data) {
    client_info *client = (client_info *)client_data;
    int sock = client->socket;
    char buffer[BUFFER_SIZE];
    int len;

    while ((len = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[len] = '\0';
        printf("Received message from %s: %s\n", client->username, buffer);
        
        if (strcmp(buffer, "QUIT") == 0) {
            printf("User %s disconnected\n", client->username);
            break;
        } else if (strncmp(buffer, "POST", 4) == 0) {
            char message[BUFFER_SIZE];
            snprintf(message, BUFFER_SIZE, "MESG[%s] %s", client->username, buffer + 5);

            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < FD_SETSIZE; i++) {
                if (clients[i] != NULL && clients[i]->socket != sock) {
                    if (send(clients[i]->socket, message, strlen(message), 0) < 0) {
                        perror("send");
                    }
                }
            }
            pthread_mutex_unlock(&clients_mutex);

            printf("%s\n", message);
        }
    }

    if (len < 0) {
        perror("Receive failed");
    }

    close(sock);

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < FD_SETSIZE; i++) {
        if (clients[i] == client) {
            clients[i] = NULL;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    free(client_data);
    return NULL;
}
