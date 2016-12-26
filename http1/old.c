/* Dont mind this.
 * Old version of traitement_thread to have it
 * available when coding new version */
void *traitement_thread(void *arg) {
  Client *c = (Client *) arg;
  char *buffer = malloc (BUF_SIZE * sizeof (char));
  char *filename;
  int fd, n;
  char *nom = malloc (40 * sizeof (char));
  char *ext = malloc (40 * sizeof (char));
  char *lu = malloc (40 * sizeof (char));
  char fichier[40];
  struct stat st;
  char *requete = malloc (BUF_SIZE * sizeof (char));
  
  /* TODO : pb here : chaque thread va malloc
   * --> Une seule malloc necessaire, pthread_once? */
  tab_ext =  (mr_mime**) malloc (1500 * sizeof (mr_mime*));
#ifdef DEBUG
  //  char DUMMYFILENAME[] = "serveur.c";
  //filename = DUMMYFILENAME;
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

  /* garder uen copie de la requete intacte */
  strcpy (requete, buffer);

  if ( !msg_bien_forme(requete) ) {
    /* Issue with request format */
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

  /* recup le nom du fichier */
  if (pthread_mutex_lock(&mutex_strtok) != 0) {
    perror("lock mutex_strtok");
    exit(1);
  }
  strtok (buffer, "/");
  strcpy(fichier, strtok(NULL," "));
  if (pthread_mutex_unlock(&mutex_strtok) != 0) {
    perror("unlock mutex_strtok");
    exit(1);
  }
#ifdef DEBUG
  printf ("ficher : %s\n", fichier);
#endif


  /* recuperer les infos du fichier */
  stat (fichier, &st);

  
  /* recuperer info pour le fichier le log  */
  /* TODO Manage address writing in logs... */
  //SetLogAddr2 (&c -> loginfo, c->address);
  SetLogTime  (&c -> loginfo);
  SetLogPid   (&c -> loginfo);
  SetLogTid   (&c -> loginfo);
  if (pthread_mutex_lock(&mutex_strtok) != 0) {
    perror("lock mutex_strtok");
    exit(1);
  }
  SetLogLine  (&c -> loginfo, strtok (requete, "\n"));
  if (pthread_mutex_unlock(&mutex_strtok) != 0) {
    perror("unlock mutex_strtok");
    exit(1);
  }
  SetLogSret  (&c -> loginfo, 200);
  SetLogRsize (&c -> loginfo, st.st_size);
  
  /* test if FIle exist */
  /* TODO : Plus propre test existence/droits... */
  if ( (fd = open (fichier, O_RDONLY)) == -1) {
    lu = "HTTP/1.1 404 Not Found\nContent-Type: text/html\n\n<html><body>\n\n<h1>404</h1>\n<h2>Not Found</h2>\n</body></html>";
    if(send(c->sock,lu, strlen (lu)
	    , 0) < 0) {
      perror("send()");
      exit(1);
    }

    SetLogSret (&c->loginfo, 404);

    /* Write logs */
    WriteLog(&c->loginfo, NULL);

    
#ifdef DEBUG
    printf ("ficher : %s pas ouvrable\n", fichier);
#endif
    close (c->sock);

    
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

  /* tester si l'on a les bon droits sur le fichier */
  if  ( (st.st_mode & S_IRGRP) != S_IRGRP) {

        lu = "HTTP/1.1 403 FORBIDDEN\nContent-Type: text/html\n\n<html><body>\n\n<h1>403</h1>\n<h2>FORBIDDEN</h2>\n</body></html>";
    if(send(c->sock,lu, strlen (lu)
	    , 0) < 0) {
      perror("send()");
      exit(1);
    }

    SetLogSret (&c->loginfo, 403);

    /* Write logs */
    WriteLog(&c->loginfo, NULL);

    
#ifdef DEBUG
    printf ("ficher : %s pas ouvrable\n", fichier);
#endif
    
    close (c->sock);

    
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
   
  /* If file exist  */
  lu = "HTTP/1.1 200 OK\nContent-Type: ";
  /* TODO : Delay that */
  if (send (c->sock, lu, strlen (lu), 0) < 0) {
    perror("send()");
    exit(1);
  }

  
  
  /* recuperer l'extension du fichier  */
  strtok (fichier, ".");
  ext = strtok (NULL, ".");

#ifdef DEBUG
  printf ("ext : %s\n", ext);
#endif

  
  /*  remplir le tableau de type mime */
    tab_ext = parse_file( &count);
#ifdef DEBUG
  printf( "tableau des extentions rempli\n");
  fflush(stdout);
#endif
  
  /* int i = 0; */
  
  /* for ( i = 0; i < count; ++i)  { */
  /*   printf ("ext : %s, nom %s\n", tab_ext[i]->extension, */
  /* 	    tab_ext[i]->nom); */
  /*   fflush (stdout); */
  /* } */

  type_mime (tab_ext, ext , nom, count);
#ifdef DEBUG
  printf ("type mime trouvé %sxx\n", nom);
  fflush (stdout);
#endif



  
  
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