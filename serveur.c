#define _XOPEN_SOURCE 700

/* include libc  */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

/* include .h de nous  */


/* mutex pour proteger le compteur de client  */
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


/* le thread lancé pour le traitement d'un client  */
void *fn_thread_http(void *requete) {

  return NULL;
}


int main (int argc, char ** argv) {
  /* arguments passé en ligen de commande  */
  int num_port  = 0;
  int nb_client = 0;
  int num_cpt   = 0;

  /* descripteur de la socket */
  int sockd;

  /* structure qui contient les informations de la socket */
  struct sockaddr_in sinf ;

  /* nombre de client en simultané */
  int cpt = 0;

  /* verification si l'appel au programme est bon  */
  if (argc != 4) {
    perror ("mauvaise utilisation\n [port] [nbclientmax] [q5]");
    exit (1);
  }

  /* recuperation des variables  */
  num_port   = atoi (argv[1]);
  nb_client  = atoi (argv[2]);
  num_cpt    = atoi (argv[3]);


  /* creation de la socket  */
  if ( (sockd = socket (AF_INET, SOCK_DGRAM, 0)) < 0) {
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


   /* traitement re la reception des messages du serveur  */
   while (1) {


     /* acceder au compteur pour chercher si il y a assez de place
	pour un nouveau
      */     
     if (pthread_mutex_lock (&mutex) < 0) {
       perror ("lock mutex seveur");
       exit (1);
     }
     
     if (cpt < nb_client) {
       /* creer un nouveau thread  */

       /* incrementation du compteur de client  */
       cpt ++;
     }

     /* relacher le compteur  */
     if (pthread_mutex_unlock (&mutex) < 0) {
       perror ("unlock mutex seveur");
       exit (1);
     }
     
   }   
  
  /*   variable partagées dans les threads
       -> mettre un mutex pour la gestion du compteur de clients

       ->creer la socket en UDP
       ->remplir les informations
       ->faire un bind pour les lier (detruire la connextion existance 
       si il en existe une

	lire en continue de flux dentrée
       une fois un message bien formé arrive :
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
          repondre : Content-Type: (le type trouvé)

       -> envoyer la reponse au client (ip dans la requette)

       -> decrementer cpt (mutex et tout)

       -> finir le thread

    */
  
  
  return 0;
}


