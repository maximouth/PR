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
  /* arguments passé en ligen de commande  */
  int num_port  = 0;
  int nb_client = 0;
  int num_cpt   = 0;

  /* descripteur de la socket */
  int sockd;

  /* structure qui contient les informations de la socket */
  struct sockaddr_in serv;
  
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


