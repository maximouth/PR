#ifndef CLIENT_H
#define CLIENT_H

#include "server.h"

typedef struct {
	struct sockaddr_in csinf;
	int sock;
	pthread_t thread;
} Client;

#endif