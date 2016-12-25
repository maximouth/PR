#ifndef SERVER_H
#define SERVER_H

#define _XOPEN_SOURCE 700

#define ANS_SIZE 200

#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#include "logger.h"
#include "client.h"

#define FLAG_200 1
#define FLAG_403 2
#define FLAG_404 4

extern pthread_mutex_t mutex_strtok;

typedef struct{
	char request[REQ_SIZE];
	unsigned int index;
    Loginfo loginfo;
    Client *client;
    pthread_mutex_t mutex_self;
    pthread_mutex_t mutex_next;
} Request;

#endif