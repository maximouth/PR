#include "requete.h"
#define BUF_SIZE 500

void *traitement_requete (void *arg) {
  Request *r = (Request *) arg;
  /* File variables */
  char filename[50];
  char tmp[52] = "./";
  struct stat st;
  char filesize[14];
  int fd, n;
  char buffer[BUF_SIZE];
  /* Request variables */
  char header[ANS_SIZE] = "HTTP/1.1 ";
  unsigned int code_flag = 0;
  Loginfo loginfo;
  /* fichier executable */
  int exe = 0, pid, status;
  int pipe_synchro, pipe_return;
  char synchroname[13] = "synchro";
  char returnname[12] = "return";


  /** type mime variable **/
   /* tableau de type mime  */
  /* C'est quand meme super sale de devoir alouer le tableau a chaque fois... */
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
  	if (!exe) { 
		strcpy (fich, filename);
		if (pthread_mutex_lock(&mutex_strtok) != 0) {
			perror("pthread_mutex_lock(mutex_strtok)");
			exit(1);
		}
	 	extf = strtok (fich, ".");
#ifdef DEBUG
	    printf ("ext => %s \n", extf);
	    fflush (stdout);
#endif
	    extf = strtok (NULL, ".");
	    if (pthread_mutex_unlock(&mutex_strtok) != 0) {
	    	perror("pthread_mutex_unlock(mutex_strtok)");
	    	exit(1);
	    }

	 #ifdef DEBUG
	   printf ("ext => %s \n", extf);
	   fflush (stdout);
	 #endif

	   if (extf != NULL) {
	   
	   /* Get the mime type  */
	   type_mime (tab_ext, extf , nom, count);
	 #ifdef DEBUG
	   printf ("type mime trouve %s pour ext %s \n", nom, extf);
	   fflush (stdout);
	 #endif
	   }
	   else {
	     nom = "text/plain";
	   }
	}
  

  /* Set Loginfo */
  SetLogAddr (&loginfo, r->client->address);
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
  if (!exe)
  	SetLogRsize (&loginfo, st.st_size);

  /* Sets answer depending on return code
   * Only if not executable! */
  if(code_flag&FLAG_200 && !exe) {
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
  	/* Content-Length: 60 because it is the length of the html sent */
  	strcat(header, "403 Forbidden\nContent-Length: 60\n\n<html><body>\n<h1>403</h1>\n<h2>Forbidden</h2>\n</body></html>\n");
  	SetLogSret(&loginfo, 403);

  }
  else if (code_flag&FLAG_404) {
  	/* Content-Length: 60 because it is the length of the html sent */
  	strcat(header, "404 Not Found\nContent-Length: 60\n\n<html><body>\n<h1>404</h1>\n<h2>Not Found</h2>\n</body></html>\n");
  	SetLogSret(&loginfo, 404);
  }
  /* --------------------------------------------------------------------------- */
  /* Question 3 */
  if (exe) {
  	/* Create pipe for syncronisation */
  	if (mkfifo(synchroname, S_IRUSR|S_IWUSR) < 0) {
  		perror("mkfifo()");
  		exit(1);
  	}
  	/* Pipe to read return value and size of answer */
  	if (mkfifo("pipe_return", S_IRUSR|S_IWUSR) < 0) {
  		perror("mkfifo()");
  		exit(1);
  	}
  	pid = fork();
  	if (pid < 0) {
  		perror("fork()");
  		exit(1);
  	}
  	if (pid == 0) {
  		/* Fils */
  		strcat(tmp,filename);
  		execlp (tmp, tmp, NULL);
  		perror("execlp()");
  		exit(1);
  	}
  	/* Pere */
  	if ( (pipe_synchro=open("pipe_synchro", O_WRONLY)) < 0 ) {
  		perror("open()");
  		exit(1);
  	}
  	/* TODO : !!!!!!Move this at right place */
  	wait(&status);
  	if (!WIFEXITED(status) || WEXITSTATUS(status)!=0 ) {
  		SetLogSret(&loginfo, 500);
  	}
  	else {
  			/* TODO.. read/write into pipes */
  	}
  }

  /* --------------------------------------------------------------------------- */
  /* Wait previous request over */
  if (pthread_mutex_lock(&r->mutex_self) != 0) {
  	perror("mutex lock : mutex_self");
  	exit(1);
  }
  if (exe) {
  	write(pipe_synchro, "Continue\0",8);
  }
  /* --------------------------------------------------------------------------- */
  /* Send header+file and write logs */
  if( !exe && send(r->client->sock, header, strlen(header), 0) < 0) {
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
  	/* TODO make sure this works with exec... */
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
