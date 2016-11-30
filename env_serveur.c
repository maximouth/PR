#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

char* get(char *name){
  return getenv(name);
}

int set(char *name, char *val){
  return setenv(name, val, 1);
}

int main (int argc, char ** argv) {
  struct sockaddr_in serv;
  struct sockaddr_in exp;
  unsigned int fromlen = sizeof (exp);
  char host [64];
  char message [80];
  char *tmp;
  int cpt = 0;
  int sd;
  int retSet;

  
  
  /* creer la socket  */
  if ( (sd = socket (AF_LOCAL, SOCK_DGRAM, 0 )) < 0) {
    perror  ("socket");
    exit (1);
  }

  /* remplir le nom   */
  memset ((char*) &serv, 0, sizeof(serv));
  serv.sin_addr.s_addr = htonl (INADDR_ANY);
  serv.sin_port = htons ( atoi (argv[1] ) );
  serv.sin_family = AF_LOCAL;

  /* nomage */
  if (bind (sd, (struct sockaddr *) &serv, sizeof(serv)) < 0) {
    perror ("bind");
    exit (1);
  }

  /* reception  */
  while (1) {

    if (recvfrom (sd, message, sizeof (message), 0,
		  (struct sockaddr*) &exp, &fromlen
		  )
	== -1) {
      perror ("recvfrom");
      exit (1);
    }

    if((tmp=malloc(15*sizeof(char)))==NULL){
      perror("malloc");
      exit(1);
    }
    tmp = strtok(message, " ");
    if(strcmp(tmp,"S") == 0){
      /*Traitement set*/
      retSet = set (strtok(NULL," ") ,strtok(NULL," ") );
      /*Renvoie set*/
      if( sendto(sd,&retSet, sizeof(retSet),0,
		 (struct sockaddr *)&exp, fromlen) == -1){
	perror("sendto");
	exit(1);
      }
    }
    else
      if(strcmp(tmp ,"G") == 0){
	/*Traitement get*/
	tmp = get (strtok(NULL," ") );
	/*Renvoie get*/
	if(sendto(sd, tmp, 15*sizeof(char),0,
		  (struct sockaddr *)&exp, fromlen) ==-1){
	  perror("sendto");
	  exit(1);
	}
      }
  } 
  return 0;
}

