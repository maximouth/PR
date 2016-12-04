#define _XOPEN_SOURCE 700

/* include libc  */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <pthread.h>

/* include .h de nous  */


/* mutex pour proteger le compteur de client  */
static pthread_mutex_t mutex_cpt = PTHREAD_MUTEX_INITIALIZER;

/* mutex pour proteger le compteur de client  */
static pthread_mutex_t mutex_thread = PTHREAD_MUTEX_INITIALIZER;

/* nombre de client en simultané */
int cpt = 0;

/* rajouter un mutex_cpt pour l'utilisation du tableau */
/* tableau des threads libre  */
  int *free_thread;
  


/* le thread lancé pour le traitement d'un client  */
void *traitement_client(void *requete) {
  int ind = 0;

#ifdef DEBUG
  printf ("dans le thread\n");
#endif

  /* prendre le compteur  */
  if (pthread_mutex_lock (&mutex_cpt) < 0) {
    perror ("unlock mutex_cpt thread");
    exit (1);
  }

  /* liberer de la place pour un nouveau client*/
  cpt --;

  /* gerer le tableau des threads remettre le bon indice a 0 */

  /* prednre le tableau  */
  if (pthread_mutex_lock (&mutex_thread) < 0) {
    perror ("unlock mutex_thread thread");
    exit (1);
  }

  /*dernier char de requete = indice*/
  free_thread [ind] = 0;

  /* rendre le tableau  */
  if (pthread_mutex_unlock (&mutex_thread) < 0) {
    perror ("unlock mutex_thread thread");
    exit (1);
  }

  
  /* relacher le compteur  */
  if (pthread_mutex_unlock (&mutex_cpt) < 0) {
    perror ("unlock mutex_cpt thread");
    exit (1);
  }

  return NULL;
}



/* test si le message est bien formé ou non  */
int msg_bien_forme (char *buff, int taille) {
  if ( (buff[taille-1] == '\n') && (buff[taille-2] == '\n') )
    return 1;
  else return 0;
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

  /* taille lu de rcvfrom  */
  int n = 0;

  /* buffer ou l'on stocke les message lu  */
  char buff [201];

  /* tableau de thread  */
  pthread_t *pthread ;
  
  /* indice du premier thread libre dans le tableau  */
  int ind = 0;

  /* verification si l'appel au programme est bon  */
  if (argc != 4) {
    perror ("mauvaise utilisation\n [port] [nbclientmax] [q5]");
    exit (1);
  }
 
  /* recuperation des variables  */
  num_port   = atoi (argv[1]);
  nb_client  = atoi (argv[2]);
  num_cpt    = atoi (argv[3]);

  /* creation du tableau de thread en fonction du nombre de client */
  pthread = malloc (nb_client * sizeof (pthread_t));
  free_thread = malloc (nb_client * sizeof (int));
  
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

     /* stocker le message lu dans buff
     	et regarder si buff n-1 et n-2 == \n
	-> message bien formé
	
	si bien formé passer a la suite
	recommencer

	while sur le resultat d'un fonction ?
     */

#ifdef DEBUG
     printf( "serveur en ecoute\n");
#endif
     
     n = recvfrom (sockd, buff, 200, 0, NULL, NULL);
     
     while (msg_bien_forme (buff,n) != 1) {
       n = recvfrom (sockd, buff, 200, 0, NULL, NULL);
     }

#ifdef DEBUG
     printf( "reception d'un message bien formé\n");
#endif

     /* acceder au compteur pour chercher si il y a assez de place
	pour un nouveau
      */     
     if (pthread_mutex_lock (&mutex_cpt) < 0) {
       perror ("lock mutex_cpt seveur");
       exit (1);
     }
     
     if (cpt < nb_client) {

       /* gestion des thread libre ou pas...
	  le plus simple un tableau (pas efficace ni rien mais bon
	*/
       ind = 0;

     /* prednre le tableau  */
     if (pthread_mutex_lock (&mutex_thread) < 0) {
       perror ("lock mutex_thread seveur");
       exit (1);
     }

     while (free_thread[ind] == 1) {
       ind ++;
     }
     /* le marqué comme pris */
     free_thread[ind] = 1;
     
     /* relacher le tableau  */
     if (pthread_mutex_unlock (&mutex_thread) < 0) {
       perror ("unlock mutex_thread seveur");
       exit (1);
     }

       
       /* pour pouvoir recuperer l'indice du tableau  */
       buff [200] = ind;
       
       /* creer un nouveau thread  */
       if ( pthread_create(&pthread[ind], NULL, traitement_client,
			   buff) == -1) {
	 perror("pthread_create");
	 exit (1);
       }
       
       
       /* incrementation du compteur de client  */
       cpt ++;
#ifdef DEBUG
       printf ("NBCLIENT : %d\n", cpt);
#endif
     }

     /* relacher le compteur  */
     if (pthread_mutex_unlock (&mutex_cpt) < 0) {
       perror ("unlock mutex_cpt seveur");
       exit (1);
     }
     
   }    
   
  /*   variable partagées dans les threads
       -> mettre un mutex_cpt pour la gestion du compteur de clients

       ->creer la socket en UDP
       ->remplir les informations
       ->faire un bind pour les lier (detruire la connextion existance 
       si il en existe une

       -->lire en continue de flux dentrée :

       ---> attendre d'avoir un message bien formé dans le buffer
                GET /chemin HTTP/1.1\nHost: XXX.X.X.X\n\n  
	   -> le retirer du buffer (chiant... )
	        --> copier de buff [size] a size max dans buff [0] 
		    puis remplir le reste de 0 
		    laisser l'offset du tableau a offset - size 

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

       -> decrementer cpt (mutex_cpt et tout)

       -> finir le thread

    */
  
  
  return 0;
}


