#ifndef CLIENT_H
#define CLIENT_H

#include "mynet.h"

void run_client(const char *username, int port, struct sockaddr_in *from_adrs);

#endif  // CLIENT_H