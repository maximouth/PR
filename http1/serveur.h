#ifndef SERVER_H
#define SERVER_H

#define _XOPEN_SOURCE 700

/* include libc  */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>


/* defines */
#define BUF_SIZE 700

/* mutex pour proteger le compteur de client  */
static pthread_mutex_t mutex_thread = PTHREAD_MUTEX_INITIALIZER;

/* mutex pour proteger le compteur de client  */
static pthread_mutex_t mutex_cpt = PTHREAD_MUTEX_INITIALIZER;

/* mutex pour proteger strtok qui n'est pas thread-safe
 * /!\ TOUS les appels a strtok sur une chaine donnee,
 * DOIVENT etre fait au sein de la meme section critique! */
pthread_mutex_t mutex_strtok = PTHREAD_MUTEX_INITIALIZER;


/* nombre de client en simultané */
int cpt = 0;



int *free_client;
  
/* Prototypes */
//void *traitement_client(void *client);
void *traitement_client(void *arg);
int msg_bien_forme (char *buff);

#endif
