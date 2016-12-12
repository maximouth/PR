#ifndef CLIENT_H
#define CLIENT_H

#include "serveur.h"
#include "logger.h"

typedef struct {
	struct sockaddr_in csinf;
	int sock;
	pthread_t thread;
	int index;
	Loginfo loginfo;
} Client;

#endif
