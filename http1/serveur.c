#include <fcntl.h>

/* include .h de nous  */
#include "serveur.h"
#include "parse.h"
#include "requete.h"


#define CL_BUSY 1
#define CL_FREE 0

/* nombre de client en actuel */
int cpt_client = 0;

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
  int cont_flag;
  /* List of clients */
  Client *clients;
  /* Buffer  for STDIN reading*/
  char buffer[BUF_SIZE];

  int true;

  
  
  /* verification si l'appel au programme est bon  */
  if (argc != 4) {
    fprintf(stderr, "mauvaise utilisation :%s [port] [nbclientmax] [q5]\n", argv[0]);
    exit (1);
  }
  
  /* recuperation des parametres en entree  */
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

  /* Remplissage de la struct d'info */
  memset ((char*) &sinf, 0, sizeof(sinf));
  sinf.sin_addr.s_addr = htonl(INADDR_ANY);
  sinf.sin_port = htons ( num_port );
  sinf.sin_family = AF_INET;
  /* MAYBE we need : 
     true = 1;
     setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int))
  */
  true = 1;
  setsockopt(sockd, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int));

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
  
  printf ("Serveur en ecoute\n");
  printf ("Le serveur peut etre stope en tapant \"QUIT\" ou \"quit\" suivi de ENTREE\n");

  cont_flag = 1;

  while(cont_flag) {
    /* Clear rdfs */
    FD_ZERO(&rdfs);
    /* Add stdin and sockd to rdfs */
    FD_SET(STDIN_FILENO, &rdfs);
    FD_SET(sockd, &rdfs);
    /* Select from rdfs */
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

      if ( strncmp("QUIT\n", buffer, 5) == 0
	  || strncmp("quit\n", buffer, 5) == 0){
      	cont_flag = 0;
      }
      else
      	printf("Commande inconnue\n");
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
      printf("Connection attempt : \n");
      printf("\t > Client address : %s\n", inet_ntoa(csin.sin_addr));
      fflush(stdout);
      /* Lock mutex */
      if (pthread_mutex_lock (&mutex_cpt) < 0) {
	       perror ("lock mutex_cpt seveur");
	       exit (1);
      }
      /* Server is able to accept more clients */
      if (cpt_client < nb_client) {
	 cpt_client ++;

	/* client en traitement  */
	printf ("Traitement d'un client...");
		
	ind = 0;
	/* prendre le tableau  */
	if (pthread_mutex_lock (&mutex_thread) < 0) {
	  perror ("lock mutex_thread (serveur)");
	  exit (1);
	}
	while (free_client[ind] == CL_BUSY) {
	  ind ++;
	}
	/* le marquer comme pris */
	free_client[ind] = CL_BUSY;
	/* relacher le tableau  */
	if (pthread_mutex_unlock (&mutex_thread) < 0) {
	  perror ("unlock mutex_thread (seveur)");
	  exit (1);
	}

	clients[ind].sock = client_sock;
	clients[ind].index = ind;
	strncpy(clients[ind].address, inet_ntoa(csin.sin_addr), 15);
	
	/* Create pthread associated with the new client */
	fflush(stdout);
	if ( pthread_create(&(clients[ind].thread), NULL, traitement_client,
			    &(clients[ind])) == -1) {
	  perror("pthread_create");
	  exit (1);
	}
  printf(" Fin du traitement du client.\n");

      } /* end if cpt < nb_client */
      else {
        printf("Surcharge de clients\n");
      }

      /* relacher le compteur  */
      if (pthread_mutex_unlock (&mutex_cpt) < 0) {
	perror ("unlock mutex_cpt (seveur)");
	exit (1);
      }
    } /* End case input on connection socket */

  } /* End while(cont_flag) */

  printf("Terminaison du serveur en cours...\n");
  
  /* TODO : Join pthread before terminating? */
  close(sockd);
  return 0;
}



/* teste si le message est bien formé ou non  */
int msg_bien_forme (char *s) {
  int l = strlen(s);
  char line[BUF_SIZE] = "0";

  /* check if the request start with "GET /"  */
  if (strncmp(s, "GET /", 5) != 0) {
    /* Does not begin with "GET" */
    return -1;    
  }

  /* Lock mutex_strtok */
  if (pthread_mutex_lock(&mutex_strtok) != 0) {
    perror("lock mutex_strtok");
    exit(1);
  }
  /* get the first line of the request  */
  strncpy(line, strtok(s,"\n"), BUF_SIZE);

  /* Unlock mutex_strtok */
  if (pthread_mutex_unlock(&mutex_strtok) != 0) {
    perror("unlock mutex_strtok");
    exit(1);
  }

  /* get the length of the first line  */
  l = strlen(line);

  /* check if it is a HTTP 1.1 or HTTP 1.1 request  */
  if ( strcmp(line+l-8,"HTTP/1.1") == 0 )
    /* HTTP/1.1 case */
    return 1;

  if ( strcmp(line+l-8,"HTTP/1.0") == 0)
    /* HTTP/1.0 case */
    return 0;

  return -1;
}


/* The server starts a pthreads each time it receives a new client.
 * This thread will also start new thread, every time it receives a request
 * Those threads need to be synchronized in order to answer in the order the
 * request were sent.
 */
void *traitement_client(void *arg){
  Client *c = (Client *) arg;
  char buffer[BUF_SIZE];
  int rcv_val, msg_val, i;
  Request *first;
  Request *current, *tmp;
  pthread_t *list=NULL;
  int lengthList = 0;

#ifdef DEBUG
  printf("In client thread, threadID : %lu\n", pthread_self());
  fflush(stdout);
#endif

  /* Setup client */
  pthread_mutex_init(&c->mutex_nbRequest,NULL);
  c->nbRequest = 0;
  c->reqOver = 1;

  /* Setup request */
  if ( (first = calloc(1, sizeof(Request))) == NULL) {
    perror("calloc()");
    exit(1);
  }
  current = first;
  memset(current, 0, sizeof(Request));


  /* put the condition to true to enter the loop  */
  msg_val = 1;
  
  /* While client sends requests 
     evaluation progressive pour le test pour pas avoir 
     de probleme de lecture bloquante
   */
  while ( (msg_val == 1) &&
	  ((rcv_val = recv(c->sock, buffer, BUF_SIZE-1, 0)) > 0)
	   ) {

    if ( (msg_val = msg_bien_forme (buffer)) < 0) {
      /* Request does not match expected format */
      printf("The request does not match expected format\n");
      fflush(stdout);

      /* renvoyer le code erreur  */
      send(c->sock,"HTTP/1.1 400 Bad Request\nContent-Type: text/html\n COntent-Length:58\n <html>\n<body>\n<h1>Mauvaise requete</h1></body>\n</html>",120, 0);       
      break;
    }

    /* Request is correctly formatted */
    /* Setup Request structure */
    strncpy(current->request, buffer, REQ_SIZE);
    pthread_mutex_init(&current->mutex_self, NULL);
    if (pthread_mutex_lock(&c->mutex_nbRequest) != 0) {
      perror("pthread_mutex_lock(mutex_nbRequest");
      exit(1);
    }
    current->index = c->nbRequest ++;
    if (! c->reqOver) {
      /* Previous request not over yet
       * This one will have to wait before sending message to client */
      if (pthread_mutex_lock(&current->mutex_self) != 0) {
        perror("pthread_mutex_lock(mutex_self)");
        exit(1);
      }
    }
    if (pthread_mutex_unlock(&c->mutex_nbRequest) != 0) {
      perror("pthread_mutex_unlock(mutex_nbRequest");
      exit(1);
    }
    current->client = c;
    if ( (current->next=calloc(1, sizeof(Request))) == NULL) {
      perror("calloc()");
      exit(1);
    }
    memset(current->next, 0, sizeof(Request));
    tmp = current;
    current = current->next;

    /* Increases size of list of threads */
    lengthList++;
    if ( (list=realloc(list, lengthList*sizeof(Request))) == NULL) {
      perror("realloc() size of thread");
      exit(1);
    }
    /* Create thread */
    if (pthread_create(&list[lengthList-1], NULL, traitement_requete, (void *) tmp) != 0) {
      perror("pthread_create");
      exit(1);
    }
  }
  if (rcv_val < 0) {
    perror("recv()");
    exit(1);
  }

  /* Join threads */
  for (i=0; i<lengthList; i++) {
    if (pthread_join(list[i], NULL) != 0) {
      perror("pthread_join()");
      exit(1);
    }
  }
  free(list); list=NULL;
  current = first;
  /*while(current) {
    tmp = current->next;
    free(current);
    current = tmp
  }*/

  /* Ends connection */ 
  close(c->sock);
  printf("Closing connection client %d\n",c->index);
  fflush(stdout);

  if (pthread_mutex_lock(&mutex_cpt) != 0) {
    perror("pthread_mutex_lock(mutex_cpt)");
    exit(1);
  }
  cpt_client--;
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

  pthread_exit((void*)NULL);
  return NULL;
}
