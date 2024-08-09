#include "mynet.h"

#define USERNAME_MAXLEN 15
#define DEFAULT_PORT 50001
#define MAX_REPEAT 3

int main(int argc, char *argv[]) {
    int sock;
    in_port_t port = DEFAULT_PORT;
    struct sockaddr_in broadcast_addr, from_addr;
    if (argc < 2 || argc > 3) {
        frpintf(stderr, "Usage: %s <username> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char username[USERNAME_MAXLEN];
    strncpy(username, argv[1], USERNAME_MAXLEN);

    if (argc == 3) port = atoi(argv[2]);
}