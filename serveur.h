#ifndef SERVER_H
#define SERVER_H

#define _XOPEN_SOURCE 700

/* include libc  */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#include <pthread.h>

/* include .h de nous  */
#include "parse.h"

/* defines */
#define BUF_SIZE 701

/* mutex pour proteger le compteur de client  */
static pthread_mutex_t mutex_thread = PTHREAD_MUTEX_INITIALIZER;

/* mutex pour proteger le compteur de client  */
static pthread_mutex_t mutex_cpt = PTHREAD_MUTEX_INITIALIZER;


/* nombre de client en simultané */
int cpt = 0;



int *free_client;
  
/* Prototypes */
void *traitement_client(void *client);
void *traitement_thread(void *arg);
int msg_bien_forme (char *buff, int taille);

#endif
