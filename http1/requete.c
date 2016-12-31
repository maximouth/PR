#include "requete.h"
#define BUF_SIZE 500

void *traitement_requete (void *arg) {
  Request *r = (Request *) arg;
  /* File variables */
  char filename[50];
  char tmp[52] = "./";
  struct stat st;
  char filesize[14];
  int fd, n, i;
  char buffer[BUF_SIZE];
  /* Request variables */
  char header[ANS_SIZE] = "HTTP/1.1 ";
  unsigned int code_flag = 0;
  Loginfo loginfo;
  /* fichier executable */
  int exe = 0, pid, status;
  int pipe_synchro, pipe_return;
  char *synchroname = malloc (20 * sizeof (char));
  char *returnname = malloc (20 * sizeof (char));
  char *lu = malloc (2 * sizeof (char));
  char *pidchar = malloc (6 * sizeof (char));

  synchroname = strcat (synchroname, "synchro");
  returnname  = strcat (returnname,  "return");
    
  /** type mime variable **/
  /* tableau de type mime  */
  /* C'est quand meme super sale de devoir alouer le tableau a chaque fois... 
     
     le free est rajouté, c'est moins sale du coup :p
     
 */
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

    /* get the pid  */
    pid = pthread_self();
    sprintf (pidchar, "%d", pid);
    /* get a unique pipe name  */
    synchroname = strcat (synchroname, pidchar);
    returnname  = strcat (returnname, pidchar);      
    
    /* Create pipe for syncronisation */
    if (mkfifo(synchroname, S_IRUSR|S_IWUSR) < 0) {
      perror("mkfifo() SYNCHRO");
      exit(1);
    }
    /* Pipe to read return value and size of answer */
    if (mkfifo(returnname, 0760 ) < 0) {//S_IRUSR|S_IWUSR) < 0) {
      perror("mkfifo() RETURN ");
      exit(1);
    }

    pid = fork();
    if (pid < 0) {
      perror("fork()");
      exit(1);
    }

    if (pid == 0) {
      /***** Fils ******/

#ifdef DEBUG
      printf ("dans le fils\n");
      fflush (stdout);	
#endif

      /* open the two pipes, 
	    -> one for synchro, 
	    -> one for the return value of exclp
      */ 
      if ( (pipe_synchro = open(synchroname, O_RDONLY)) < 0 ) {
	perror("open() fils synchro");
	exit(1);
      }
      
      if ( (pipe_return = open(returnname,  O_WRONLY)) < 0 ) {
	perror("open() fils return");
      exit(1);
      }

#ifdef DEBUG
      printf ("pipe open fils\n");
      fflush (stdout);	
#endif


/*       /\* wait fot the green signal  *\/ */
/*       read (pipe_synchro, lu, 1); */
/*       while (lu[0] != '1') { */
/* #ifdef DEBUG */
/*       printf ("waiting fot the green light\n"); */
/*       fflush (stdout);	 */
/* #endif */
/* 	read (pipe_synchro, lu, 1); */
/*       } */
      
      /* redirect the standar output on the pipe return  */
      dup2 (pipe_return, STDOUT_FILENO);

      /* execute the new program */
      strcat(tmp,filename);
      execlp (tmp, tmp, NULL);
      perror("execlp()");
      exit(1);
    }
    

#ifdef DEBUG
    printf ("dans le pere\n");
    fflush (stdout);	
#endif
    
    
    /***** Pere ******/

    /* open the pipe */
    if ( (pipe_synchro = open (synchroname, O_WRONLY)) < 0 ) {
      perror("open() pere synchro");
      exit(1);
    }
    if ( (pipe_return = open(returnname,  O_RDONLY)) < 0 ) {
      perror("open() pere return");
      exit(1);
    }

#ifdef DEBUG
      printf ("pipe open pere\n");
      fflush (stdout);	
#endif

    
    /* TODO : !!!!!!Move this at right place 
       I don't know where..
     */
    wait(&status);
    if ( !WIFEXITED(status) || WEXITSTATUS(status)!=0 ) {
      SetLogSret(&loginfo, 500);
    }
    else {
      /* /\* send the green light to the son *\/ */
      /* send(pipe_synchro, "11111", 5, 0); */
      
      /* send to the client all of the son write on stdout */
      while (read (pipe_return, lu, 1) != 0) {
	send (r->client->sock, lu, 1, 0);
      }
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

    free (nom);
    free (extf);
    free (fich);
   
  for (i = 0 ; i < 1500 ; i++) {
    free (tab_ext[i]);
  }
  free (tab_ext);
#ifdef DEBUG
  printf ("all ressources free");
#endif
  
  return NULL;
}
