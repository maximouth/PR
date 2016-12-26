#include <fcntl.h>

#include "client.h"
#include "serveur.h"
#include "parse.h"


#define CL_BUSY 1
#define CL_FREE 0

/* tableau de type mime  */
mr_mime** tab_ext;

/* number of type mime trouve  */
int count = 0;

int main (int argc, char ** argv) {
  /* arguments passé en ligne de commande  */
  int num_port  = 0;
  int nb_client = 0;
  int num_cpt   = 0;
  /* descripteur de la socket de connexion */
  int sockd;
  /* structure qui contient les informations de la socket de connexion */
  struct sockaddr_in sinf;
  /* Set of socket descriptor for select */
  fd_set rdfs;
  /* indice du premier thread libre dans le tableau */
  int ind = 0;
  /* stop flag for the server */
  int stop_flag;
  /* List of clients */
  Client *clients;
  /* Buffer  for STDIN reading*/
  char buffer[BUF_SIZE];

  
  
  /* verification si l'appel au programme est bon  */
  if (argc != 4) {
    fprintf(stderr, "mauvaise utilisation :%s [port] [nbclientmax] [q5]\n", argv[0]);
    exit (1);
  }
  
  /* recuperation des variables  */
  num_port   = atoi (argv[1]);
  nb_client  = atoi (argv[2]);
  num_cpt    = atoi (argv[3]);

  /* creation du tableau de thread en fonction du nombre de client */
  free_client = calloc (sizeof(int), nb_client);
  clients = calloc (sizeof(int), nb_client);
  if(free_client == NULL || clients == NULL){
    perror("calloc clients/free_client");
    exit(1);
  }
  
  /* creation de la socket  */
  if ( (sockd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
    perror ("socket()");
    exit (1);
  }

  /* creation de l'interface */
  memset ((char*) &sinf, 0, sizeof(sinf));
  sinf.sin_addr.s_addr = htonl(INADDR_ANY);
  sinf.sin_port = htons ( num_port );
  sinf.sin_family = AF_INET;
  /* MAYBE we need : 
  true = 1;
  setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int))
  */


  /* bind entre le descipteur de socket et la structure  */
  if (bind (sockd, (struct sockaddr *) &sinf, sizeof(sinf)) < 0) {
    perror ("bind");
    exit (1);
  }

  /* Listen onto the socket */
  if (listen(sockd, nb_client) < 0) {
    perror ("listen()");
    exit(1);
  }

#ifdef DEBUG
  printf( "End server setup.\n");
  fflush(stdout);
#endif
  
  printf ("Serveur en ecoute\n");
  printf ("Le serveur peut etre stope en tapant \"QUIT\" suivi de ENTREE\n");
  
  
  stop_flag = 1;

  while(stop_flag) {
    /* Clear rdfs */
    FD_ZERO(&rdfs);

    /* Add stdin and sockd to rdfs */
    FD_SET(STDIN_FILENO, &rdfs);
    FD_SET(sockd, &rdfs);

    /* Select from rdfs *
     * Use of select in order to be able  *
     * to receive commands from STDIN */
    if (select(sockd+1, &rdfs, NULL, NULL, NULL) < 0) {
      perror("select()");
      exit(1);
    }

    /* Input on STDIN */
    if (FD_ISSET(STDIN_FILENO, &rdfs)) {
#ifdef DEBUG
      printf("Input on STDIN\n");
      fflush(stdout);
#endif
      if(read(STDIN_FILENO, buffer, BUF_SIZE)<0) {
      	perror("read() STDIN");
      	exit(1);
      }

      if(strncmp("QUIT\n",buffer,5)==0 || strncmp("quit\n", buffer,5)==0){
      	stop_flag = 0;
      }
      else
      	printf("Unknown command\n");
      continue;
    }

    /* Connection attempt */
    if (FD_ISSET(sockd, &rdfs)) {
      /* Client socket information */
      struct sockaddr_in csin;
      int client_sock;
      socklen_t sinsize = sizeof(csin);
      memset ((char*) &csin, 0, sizeof(csin));
      /* Accept first connection onto socket */
      if ((client_sock = accept (sockd, (struct sockaddr *) &csin, &sinsize)) < 0) {
	perror("accept()");
	exit(1);
      }

#ifdef DEBUG
      printf("Connection attempt\n");
      printf("Client address : %s\n", inet_ntoa(csin.sin_addr));
      fflush(stdout);
#endif
      /* Lock mutex */
      if (pthread_mutex_lock (&mutex_cpt) < 0) {
	perror ("lock mutex_cpt seveur");
	exit (1);
      }
      
      /* Server is able to accept more clients */
      if (cpt < nb_client) {
	cpt ++;

	/* client en traitement  */
	printf ("client recu \n");
      
		
	ind = 0;
	/* prendre le tableau  */
	 if (pthread_mutex_lock (&mutex_thread) < 0) {
	 perror ("lock mutex_thread seveur");
	 exit (1);
	 }
	while (free_client[ind] == CL_BUSY) {
	  ind ++;
	}
	/* le marquer comme pris */
	free_client[ind] = CL_BUSY;
	/* relacher le tableau  */
	if (pthread_mutex_unlock (&mutex_thread) < 0) {
	   perror ("unlock mutex_thread seveur");
	   exit (1);
	}

	clients[ind].sock = client_sock;
	clients[ind].index = ind;
  strncpy(clients[ind].address, inet_ntoa(csin.sin_addr), 15);
	//clients[ind].csinf = csin;
	
	/* Create pthread associated with the new client */
	fflush(stdout);
	if ( pthread_create(&(clients[ind].thread), NULL, traitement_client,
			    &(clients[ind])) == -1) {
	  perror("pthread_create");
	  exit (1);
	}

      } /* end if cpt < nb_client */

      /* relacher le compteur  */
      if (pthread_mutex_unlock (&mutex_cpt) < 0) {
	perror ("unlock mutex_cpt seveur");
	exit (1);
      }
    } /* End case input on connection socket */
  } /* End main loop -> while(stop_flag) */

  /* TODO : Join pthread before terminating? */
  close(sockd);
  return 0;
}

/* test si le message est bien formé ou non  */
int msg_bien_forme (char *s) {
  int l = strlen(s);
  char line[BUF_SIZE];
  if ( strcmp(s+l-2, "\n\n") != 0 && strcmp(s+l-4, "\r\n\r\n"))
    /* Does not end with "\n\n" or "\r\n\r\n" */
    return 0;
  if (strncmp(s, "GET /", 5) != 0)
    /* Does not begin with "GET" */
    return 0;
  /* Lock mutex_strtok */
  if (pthread_mutex_lock(&mutex_strtok) != 0) {
    perror("lock mutex_strtok");
    exit(1);
  }
  strncpy(line, strtok(s,"\n"), BUF_SIZE);
  /* Unlock mutex_strtok */
  if (pthread_mutex_unlock(&mutex_strtok) != 0) {
    perror("unlock mutex_strtok");
    exit(1);
  }
  l = strlen(line);
  if (strcmp(line+l-8,"HTTP/1.1")!=0 && strcmp(line+l-8,"HTTP/1.0")!=0)
    /* First line does not end with "HTTP/1.0" or "HTTP/1.1" */
    return 0;
  return 1;
}




/*
  * The server starts a pthreads each time it receives a new client.
  * This thread will also start new thread, every time it receives a request
  * Those threads need to be synchronized in order to answer in the order the
  * request were sent.
*/
void *traitement_client(void *arg){
  Client *c = (Client *) arg;
  char buffer[BUF_SIZE];
  int rcv_val;
  Request first;
  Request *current, *tmp;
  pthread_t *list;
  int lengthList = 1;

#ifdef DEBUG
  printf("In client thread, threadID : %lu\n", pthread_self());
  fflush(stdout);
#endif

  /* Setup client */
  pthread_mutex_init(&c->mutex_nbRequest,NULL);
  c->nbRequest = 0;

  /* Setup request */
  current = &first;
  memset(current, 0, sizeof(Request));

  /* Setup list thread */
  if ( (list=calloc(lengthList,sizeof(pthread_t)))==NULL ) {
    perror("calloc()");
    exit(1);
  }

  /* While client sends requests */
  while ( (rcv_val = recv(c->sock, buffer, BUF_SIZE-1, 0)) > 0) {
    if (!msg_bien_forme(buffer)) {
      /* Request does not match expected format */
      close(c->sock);
      printf("The request does not match expected format\n");
      fflush(stdout);
      /* TODO : we need to chose whether we simply ignore improperly
       * formatted request, or if we send() a message to the client */
      continue;
    }
    /* Request is correctly formatted */
    if (strncmp(buffer,"GET",3)==0) {
      /* Setup Request structure */
      strncpy(current->request, buffer, REQ_SIZE);
      if (pthread_mutex_lock(&c->mutex_nbRequest) != 0) {
        perror("pthread_mutex_lock(mutex_nbRequest");
        exit(1);
      }
      current->index = c->nbRequest ++;
      if (pthread_mutex_unlock(&c->mutex_nbRequest) != 0) {
        perror("pthread_mutex_unlock(mutex_nbRequest");
        exit(1);
      }
      current->client = c;
      pthread_mutex_init(&current->mutex_self, NULL);
      /* TODO : lock mutex in case it is the first thread created
       * OR last thread already finished */
      if ( (current->next=calloc(1, sizeof(Request))) == NULL) {
        perror("calloc()");
        exit(1);
      }
      tmp = current;
      current = current->next;

      /* Increases size of list of threads */
      if ( (list=realloc(list, ++lengthList)) == NULL) {
        perror("realloc()");
        exit(1);
      }
      /* Create thread */
      if (pthread_create(&list[lengthList-1], NULL, traitement_requete, (void *) tmp) != 0) {
        perror("pthread_create");
        exit(1);
      }
    }
    else {
      /* Case we need to fork and exec --> Q3 */
      /* TODO : QUESTION 3 */
      /* Idea : Use an other keyword than GET in request
       * update msb_bien_forme according to new keyword */
    }
  }
  if (rcv_val < 0) {
    perror("recv()");
    exit(1);
  }
  /* TODO : CLEANUP JOB 
   * EVERY. SINGLE. CALLOC.
   * Follow chained list to do so */

  /* TODO : handle end of connection
   * Diminish number of clients and so on */
  return NULL;
}


/*
if ( !msg_bien_forme(requete) ) {
    close (c->sock);
    printf("The request does not match expected format\n");
    fflush(stdout);

  if (pthread_mutex_lock(&mutex_cpt) != 0) {
    perror("pthread_mutex_lock(mutex_cpt)");
    exit(1);
  }
  cpt--;
  if (pthread_mutex_unlock(&mutex_cpt) != 0) {
    perror("pthread_mutex_unlock(mutex_cpt)");
    exit(1);
  }
  if(pthread_mutex_lock(&mutex_thread) != 0) {
    perror("pthread_mutex_lock(mutex_thread");
    exit(1);
  }
  free_client[c->index] = CL_FREE;
  if(pthread_mutex_unlock(&mutex_thread) != 0) {
    perror("pthread_mutex_unlock(mutex_thread");
    exit(1);
  }

    return NULL;
  }
  */