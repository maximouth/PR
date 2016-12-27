#include "requete.h"
#define BUF_SIZE 500

void *traitement_requete (void *arg) {
  Request *r = (Request *) arg;
  /* File variables */
  char filename[50];
  char ext[10];
  struct stat st;
  char filesize[14];
  int fd, n;
  char buffer[BUF_SIZE];
  /* Request variables */
  char header[ANS_SIZE] = "HTTP/1.1 ";
  unsigned int code_flag = 0;
  Loginfo loginfo;

  /* fichier executable */
  int exe = 0;

  /** type mime variable **/
   /* tableau de type mime  */
  mr_mime** tab_ext = (mr_mime**) malloc (1500 * sizeof (mr_mime*));
   /* number of type mime trouve  */
  int count  = 0;
  char *nom  = malloc (60 * sizeof (char));
  char *extf = malloc (60 * sizeof (char));
  char *fich = malloc (50 * sizeof (char));
  
   /*  remplir le tableau de type mime */
  tab_ext = parse_file( &count);
 #ifdef DEBUG
   printf( "tableau des extentions rempli\n");
   fflush(stdout);
 #endif
  
#ifdef DEBUG
  printf("In thread request, thread ID : %lu\n", pthread_self());
#endif

  strncpy(buffer, r->request,BUF_SIZE);
  /* Gettigng filename */
  if (pthread_mutex_lock(&mutex_strtok) != 0) {
    perror("lock mutex_strtok");
    exit(1);
  }
  strtok (buffer, "/");
  strncpy(filename, strtok(NULL," "), 50);
  if (pthread_mutex_unlock(&mutex_strtok) != 0) {
    perror("unlock mutex_strtok");
    exit(1);
  }

  /* Getting file's stats.
   * Also checks if file exists
   * and if we can access it. 
   * Set flag accordingly. */
  if (stat (filename, &st) < 0) {
  	if (errno == EACCES) {
  		code_flag = FLAG_403; /* No read permission on the path*/
  		printf("HERE\n");
  		fflush(stdout);
  	}
  	else {
  		code_flag = FLAG_404; /* File does not exist */
  	}
  }
  else {
    if  (st.st_mode & S_IRUSR) {
      code_flag = FLAG_200; /* File exist and can be read */
      /*  if it is an executable file  */
      if (st.st_mode & S_IXUSR) {
	exe = 1;
      }
    }
    else
      code_flag = FLAG_403; /* No read permission on the file*/
  }


   /* Get the file extension  */
   strcpy (fich, filename);
   extf = strtok (fich, ".");
 #ifdef DEBUG
   printf ("ext => %s \n", extf);
   fflush (stdout);
 #endif

   
   extf = strtok (NULL, ".");

 #ifdef DEBUG
   printf ("ext => %s \n", extf);
   fflush (stdout);
 #endif

   if (extf != NULL) {
   
   /* Get the mime type  */
   type_mime (tab_ext, extf , nom, count);
 #ifdef DEBUG
   printf ("type mime trouvé %s pour ext %s \n", nom, extf);
   fflush (stdout);
 #endif
   }
   else {
     nom = "text/plain";
   }
  

  /* Set Loginfo */
  SetLogTime  (&loginfo);
  SetLogPid   (&loginfo);
  SetLogTid   (&loginfo);
  strncpy(buffer, r->request,BUF_SIZE);

  if (pthread_mutex_lock(&mutex_strtok) != 0) {
    perror("lock mutex_strtok");
    exit(1);
  }

  SetLogLine  (&loginfo, strtok (buffer, "\n"));

  if (pthread_mutex_unlock(&mutex_strtok) != 0) {
    perror("unlock mutex_strtok");
    exit(1);
  }
  SetLogRsize (&loginfo, st.st_size);

  /* Sets answer depending on return code */
  if(code_flag&FLAG_200) {
  	if ( (fd = open(filename, O_RDONLY) ) < 0) {
  		perror("read()");
  		exit(1);
  	}
  	strcat (header, "200 OK\n");
  	/* TODO : content type shit */
  	strcat (header, "Content-Type: ");
	strcat (header, nom);
  	strcat (header, "\nContent-Length: ");
  	sprintf (filesize,"%lu\n\n", st.st_size);
  	strcat (header, filesize);
  	SetLogSret (&loginfo, 200);
  }
  else if (code_flag&FLAG_403) {
  	/* Content-Length: 59 because it is the length of the html sent */
  	strcat(header, "403 Forbidden\nContent-Length: 59\n\n<html><body>\n<h1>403</h1>\n<h2>Forbidden</h2>\n</body></html>\n");
  	SetLogSret(&loginfo, 403);

  }
  else {
  	/* Content-Length: 59 because it is the length of the html sent */
  	strcat(header, "404 Not Found\nContent-Length: 59\n\n<html><body>\n<h1>404</h1>\n<h2>Not Found</h2>\n</body></html>\n");
  	SetLogSret(&loginfo, 404);
  }


  /* --------------------------------------------------------------------------- */
  /* Wait previous request over */
  if (pthread_mutex_lock(&r->mutex_self) != 0) {
  	perror("mutex lock : mutex_self");
  	exit(1);
  }
  /* --------------------------------------------------------------------------- */
  /* Send header+file and write logs */
  if(send(r->client->sock, header, strlen(header), 0) < 0) {
      perror("send()");
      exit(1);
  }
  if (code_flag&FLAG_200) {
  	/* Read and send whole file */
  	while ( (n = read(fd, buffer, BUF_SIZE-1)) > 0) {
  		if(send (r->client->sock, buffer, n, 0) < 0) {
  			 perror("send()");
  			 exit(1);
  		}
  	}
  	if (n < 0) {
  		perror("read()");
  		exit(1);
  	}
  	/* Close file */
  	if (close (fd) < 0) {
  		perror ("close");
  		exit (1);
  	}
  }
  WriteLog(&loginfo, NULL);
  /* --------------------------------------------------------------------------- */
  /* Unlock mutex for next request to execute */
  if (pthread_mutex_lock(&r->client->mutex_nbRequest) != 0) {
  	perror("mutex lock : mutex_nbRequest");
  	exit(1);
  }
  if (r->index < r->client->nbRequest) {
  	/* If there are more request to treat */
  	if (pthread_mutex_unlock (&r->next->mutex_self) != 0) {
  		perror("mutex unlock : mutex_next");
  		exit(1);
  	}
  }
  else {
  	/* There arent anymore request to treat */
  	r->client->reqOver = 1;
  }
  if (pthread_mutex_unlock(&r->client->mutex_nbRequest) != 0) {
  	perror("mutex unlock : mutex_nbRequest");
  	exit(1);
  }
  /* --------------------------------------------------------------------------- */
  return NULL;
}
