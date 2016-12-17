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
  /* arguments pass� en ligne de commande  */
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

  /*  remplir le tableau de type mime */
  tab_ext = parse_file( &count);
#ifdef DEBUG
  printf( "tableau des extentions rempli\n");
  fflush(stdout);
#endif

  int i = 0;
  
  for ( i = 0; i < count; ++i)  {
    printf ("ext : %s, nom %s\n", tab_ext[i]->extension,
	    tab_ext[i]->nom);
    fflush (stdout);
  }
  
  
  
  stop_flag = 1;

  while(stop_flag) {
    /* Clear rdfs */
    FD_ZERO(&rdfs);

    /* Add stdin and sockd to rdfs */
    FD_SET(STDIN_FILENO, &rdfs);
    FD_SET(sockd, &rdfs);

    printf("\t\t\t --- PRESELECT ---\n");

    /* Select from rdfs *
     * Use of select in order to be able  *
     * to receive commands from STDIN */
    if (select(sockd+1, &rdfs, NULL, NULL, NULL) < 0) {
      perror("select()");
      exit(1);
    }

    printf("\t\t\t --- POSTSELECT ---\n");

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
	/* Mutex_cpt est locke plus haut, et pas libere
	 * Celui ne sert a rien au final! 
	 * TODO : Soit relacher le compteur plus tot,
	 * soit virer ce mutex
	 if (pthread_mutex_lock (&mutex_thread) < 0) {
	 perror ("lock mutex_thread seveur");
	 exit (1);
	 }
	*/

	while (free_client[ind] == CL_BUSY) {
	  ind ++;
	}
	/* le marquer comme pris */
	free_client[ind] = CL_BUSY;
     
	/* relacher le tableau  */
	/* Meme reflexion que plus haut 
	   if (pthread_mutex_unlock (&mutex_thread) < 0) {
	   perror ("unlock mutex_thread seveur");
	   exit (1);
	   }
	*/
	clients[ind].sock = client_sock;
	clients[ind].index = ind;
	clients[ind].csinf = csin;
	/* Needs to be removed since memset(0) is done in thread
	   SetLogAddr(&clients[ind].loginfo, &csin.sin_addr);
	   printf("ADDRESS : %s\n",clients[ind].loginfo.caddr);
	   printf("ADDRESS : %d\n",clients[ind].index);
	   fflush(stdout);
	   SetLogPid(&clients[ind].loginfo);
	   printf("PID TAMERE : %s\n",clients[ind].loginfo.spid);
	   printf("PID : %d\n", (int) getpid());
	*/
	
	/* Create pthread associated with the new client */
	fflush(stdout);
	if ( pthread_create(&(clients[ind].thread), NULL, traitement_thread,
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

   
  /*   variable partag�es dans les threads
       -> mettre un mutex_cpt pour la gestion du compteur de clients

       ->creer la socket en UDP
       ->remplir les informations
       ->faire un bind pour les lier (detruire la connextion existance 
       si il en existe une

       -->lire en continue de flux dentr�e :

       ---> attendre d'avoir un message bien form� dans le buffer
       GET /chemin HTTP/1.1\nHost: XXX.X.X.X\n\n  
       -> le retirer du buffer (chiant... )
       --> copier de buff [size] a size max dans buff [0] 
       puis remplir le reste de 0 
       laisser l'offset du tableau a offset - size 

       une fois un message bien form� arrive :
       -> incrementer cpt, test et tout
       -> creer un thread avec le message en argument
       sinon rien 


       dans le thread :
       -> decoder le message
       -> chercher le fichier
       -> si il existe et accesible: 
       repondre : "HTTP/1.1 200 OK" 
       -> si il existe et pas les droit: 
       repondre : "HTTP/1.1 403 Forbidden" 
       -> si il n'existe pas: 
       repondre : "HTTP/1.1 404 not found" 
       
       -> chercher dans le fichier mime.types le typer de fichier
       repondre : Content-Type: (le type trouv�)

       -> envoyer la reponse au client (ip dans la requette)

       -> decrementer cpt (mutex_cpt et tout)

       -> finir le thread

  */
  
  close(sockd);
  return 0;
}


/* le thread lanc� pour le traitement d'un client  */
void *traitement_client(void *client) {

  Client* c = (Client *) client;
  int n;
  char buffer[BUF_SIZE];
  char *lu;
  char *fichier;
  int fd = 0;
  char *ext;
  char ret[100];
  
#ifdef DEBUG
  printf ("Dans le thread\n");
  printf("Ma socket est : %u\n", c->sock);
#endif
  if ((n = recv(c->sock, buffer, BUF_SIZE -1, 0)) < 1) {
    perror("recv()");
    exit(1);
  }

  /* Set time and tid into loginfo */
  SetLogTime(&c->loginfo);
  SetLogTid(&c->loginfo);
  
#ifdef DEBUG
  printf("Lu\n");
  printf ("requete : %s\n",buffer);
  fflush(stdout);
#endif
  if (msg_bien_forme(buffer, n)) {
    SetLogLine(&c->loginfo, strtok(buffer, "\n")); 
    /* Traitement de la requete */
#ifdef DEBUG
    printf ("Bien forme\n");
    printf ("requete : %s\n",buffer);
    fflush(stdout);
#endif
  }

  lu = strtok (buffer, "/");

  /* C'est pas le role de msg_bien_forme de faire ca? */
  if ( (strcmp (lu, "GET ")) != 0) {
    close (c->sock);
    return NULL;
  }

  fichier = strtok (NULL, " ");  
#ifdef DEBUG
  printf ("ficher : %s\n", fichier);
#endif


  /* test if FIle exist */
  if ( (fd = open (fichier, O_RDONLY)) == -1) {
    lu = "HTTP/1.1 404 Not Found\nContent-Type: text/html\n\n<html><body>\n\n<h1>404</h1>\n<h2>Not Found</h2>\n</body></html>";
    if(send(c->sock,lu, strlen (lu)
	    , 0) < 0) {
      perror("send()");
      exit(1);
    }

#ifdef DEBUG
    printf ("ficher : %s pas ouvrable\n", fichier);
#endif
    
    close (c->sock);
    return NULL;
  }

  /* If file exist  */
  lu ="HTTP/1.1 200 OK\nContent-Type: c";
  if(send (c->sock, lu,
	   strlen (lu), 0) < 0) {
    perror("send()");
    exit(1);
  }

  /* search file extension in mime type   */
  strtok (fichier, ".");
  ext = strtok (NULL, ".");

#ifdef DEBUG
  printf ("ext : %s\n", ext);
  // printf ("type_mime %s\n", type_mime (ext));
#endif

  /* recup le type mime */
  /* type_mime (ext, ret); */
  printf ("type mime : %s\n", ret);
  ////********  ICCII ******/
  /*
    if(send(c->sock, ret, strlen (ext)
    , 0) < 0) {
    perror("send()");
    exit(1);
    }
  */

  while (read (fd, buffer, BUF_SIZE) != 0) {
    if(send(c->sock,buffer, strlen (buffer)
	    , 0) < 0) {
      perror("send()");
      exit(1);
    }
      
  }
  
  /*  
      lire la requette :
      -> stocker le nom de fichier a lire
      --> GET_/nomfichier_XXXXX
      essayer de l'ouvrir
      -> si pas possible 404 
      -> si possible 200 
      
      lire type mime du fichier (fonction a cot�)
      et l'afficher

      si 200 (
      while((c = fgetc(fp)) != EOF)  // tant que l'on est pas arriv� � la fin du fichier
      if(send(c->sock, c, 1, 0) < 0) {
      perror("send() fichier");
      exit(1);
      }
      )
       
      
  */  

  /* fin du thread / connexion liberer l'espace pour les nouveau client  */

  /* prednre le tableau  */
  if (pthread_mutex_lock (&mutex_thread) < 0) {
    perror ("unlock mutex_thread thread");
    exit (1);
  }

  /*dernier char de requete = indice*/
  free_client [c->index] = CL_FREE;
#ifdef DEBUG
  printf("mutex\n");
  fflush(stdout);
#endif

  /* rendre le tableau  */
  if (pthread_mutex_unlock (&mutex_thread) < 0) {
    perror ("unlock mutex_thread thread");
    exit (1);
  }

  
  /* prednre le cpt  */
  if (pthread_mutex_lock (&mutex_cpt) < 0) {
    perror ("unlock mutex_cpt thread");
    exit (1);
  }

  /* free space for one next client  */
  cpt --;

  /* rendre le cpt  */
  if (pthread_mutex_unlock (&mutex_cpt) < 0) {
    perror ("unlock mutex_cpt thread");
    exit (1);
  }
  /* fermer la socket pour la reutiliser  */
  close (c->sock);
  /* WriteLog(c->loginfo, NULL); */
  printf("COUCOU\n");
  return NULL;
}



/* test si le message est bien form� ou non  */
int msg_bien_forme (char *buff, int taille) {
  if ( ((buff[taille-1] == '\n') && (buff[taille-2] == '\n')) || 
       /* Ends with \n\n */
       ((buff[taille-1] == '\n') && (buff[taille-2] == '\r') && 
	(buff[taille-3] == '\n') && (buff[taille-4] == '\r'))
       /* Ends with \r\n\r\n */
       )
    return 1;
  else return 0;
}


void *traitement_thread(void *arg) {
  Client *c = (Client *) arg;
  char *buffer = malloc (BUF_SIZE * sizeof (char));
  char *filename;
  int fd, n;
  char *nom = malloc (40 * sizeof (char));
  char *ext = malloc (40 * sizeof (char));
  char *lu = malloc (40 * sizeof (char));
  char *fichier = malloc (40 * sizeof (char));
  
  tab_ext =  (mr_mime**) malloc (1500 * sizeof (mr_mime*));
#ifdef DEBUG
  char DUMMYFILENAME[] = "serveur.c";
  filename = DUMMYFILENAME;
#endif

#ifdef DEBUG
  printf("In thread\n");
#endif

  /* Clean memory zone allocated to loginfo struct */
  memset(&c->loginfo, (int)'\0', sizeof(Loginfo));

  /* Server receives request */
  if (recv(c->sock, buffer, BUF_SIZE - 1, 0) < 0) {
    perror("recv()");
    exit(1);
  }
#ifdef DEBUG
  printf(">>>> Received :\n>%s\n",buffer);
  fflush(stdout);
#endif

  lu = strtok (buffer, "/");

  /* C'est pas le role de msg_bien_forme de faire ca? */
  if ( (strcmp (lu, "GET ")) != 0) {
    close (c->sock);
    return NULL;
  }

  fichier = strtok (NULL, " ");  
#ifdef DEBUG
  printf ("ficher : %s\n", fichier);
#endif


  /* test if FIle exist */
  if ( (fd = open (fichier, O_RDONLY)) == -1) {
    lu = "HTTP/1.1 404 Not Found\nContent-Type: text/html\n\n<html><body>\n\n<h1>404</h1>\n<h2>Not Found</h2>\n</body></html>";
    if(send(c->sock,lu, strlen (lu)
	    , 0) < 0) {
      perror("send()");
      exit(1);
    }

#ifdef DEBUG
    printf ("ficher : %s pas ouvrable\n", fichier);
#endif
    
    close (c->sock);
    return NULL;
  }

  /* If file exist  */
  lu ="HTTP/1.1 200 OK\nContent-Type: c";
  if(send (c->sock, lu,
	   strlen (lu), 0) < 0) {
    perror("send()");
    exit(1);
  }

  /* search file extension in mime type   */
  strtok (fichier, ".");
  ext = strtok (NULL, ".");

#ifdef DEBUG
  printf ("ext : %s\n", ext);
  // printf ("type_mime %s\n", type_mime (ext));
#endif

  
  /*  remplir le tableau de type mime */
  tab_ext = parse_file( &count);
#ifdef DEBUG
  printf( "tableau des extentions rempli\n");
  fflush(stdout);
#endif
  
  int i = 0;
  
  for ( i = 0; i < count; ++i)  {
    printf ("ext : %s, nom %s\n", tab_ext[i]->extension,
	    tab_ext[i]->nom);
    fflush (stdout);
  }

  type_mime (tab_ext, ext , nom, count);
#ifdef DEBUG
  printf("type mime trouve\n");
#endif

  printf ("type mime trouv� %s\n", nom);


  SetLogTime(&c->loginfo);
  SetLogPid(&c->loginfo);
  SetLogTid(&c->loginfo);

  /* Open file */
  if ((fd = open(filename, O_RDONLY)) < 0) {
    perror("open()");
    exit(1);
  }

  /* Read whole file, and send it to client */
  while ((n = read(fd, buffer, BUF_SIZE-1)) > 0) {
    if(send(c->sock, buffer, n, 0) < 0) {
      perror("send()");
      exit(1);
    }
  }
  if (n<0) {
    perror("read()");
    exit(1);
  }

  /* Close file */
  if (close(fd) < 0) {
    perror("close()");
    exit(1);
  }

  /* Write logs */
  WriteLog(&c->loginfo, NULL);

  /* Free ressources */
  close(c->sock);
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

