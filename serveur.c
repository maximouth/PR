#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

void *fn_thread_http(void *requete) {
  
  return NULL;
}

int main (int argc, char ** argv) {
  int num_port = 0;
  int nb_client = 0;
  int num_cpt 0;
  int sockd;
  int cpt = 0;
  struct sockaddr_in serv;
  
  
  if (argc != 4) {
    perror ("mauvaise utilisation\n");
    exit (1);
  }
  
  num_port   = atoi (argv[1]);
  nb_client = atoi (argv[2]);
  num_cpt    = atoi (argv[3]);

  /* Creation de la socket */
  /* TODO chose between AF_UNIX and AF_INET 
     unlink and/or closesocket() */
  if((sockd = socket(AF_UNIX, SOCK_DGRAM, 0)) == INVALID_SCOKET) {
      perror("Socket\n");
      exit(1);
  }
      
   /* remplir le nom   */
  memset ((char*) &serv, 0, sizeof(serv));
  serv.sin_addr.s_addr = htonl (INADDR_ANY);
  serv.sin_port = htons (num_port);
  serv.sin_family = AF_UNIX;

  unlink (
  
  
  /* nomage */
  if (bind (sockd, (struct sockaddr *) &serv, sizeof(serv)) < 0) {
    perror ("bind");
    exit (1);
  }


  
  while (1) {


  }

  
  return 0;
}


