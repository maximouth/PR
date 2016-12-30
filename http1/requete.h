#ifndef REQUETE_H
#define REQUETE_H

#define _XOPEN_SOURCE 700

#define ANS_SIZE 200
#define REQ_SIZE 200

#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "logger.h"
#include "parse.h"
//#include "client.h"

#define FLAG_200 1
#define FLAG_403 2
#define FLAG_404 4

extern pthread_mutex_t mutex_strtok;

/* I have had to move the Client struct here... */

typedef struct {
	//struct sockaddr_in csinf;
	char address[16];
	int sock;
	pthread_t thread;
	int index;
	int nbRequest;
	int reqOver;
	pthread_mutex_t mutex_nbRequest;
} Client;

struct Request {
	char request[REQ_SIZE];
	unsigned int index;
    Client *client;
    pthread_mutex_t mutex_self;
    struct Request *next;
};
typedef struct Request Request;

void *traitement_requete (void *arg);

#endif
