#ifndef CLIENT_H
#define CLIENT_H

#include "serveur.h"
#include "logger.h"

typedef struct {
	//struct sockaddr_in csinf;
	char address[16];
	int sock;
	pthread_t thread;
	int index;
    Loginfo loginfo;
} Client;

#endif
