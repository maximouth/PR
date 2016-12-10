/* include libc  */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>



void type_mime (char *ext, char* ret) {
  int fd = 0;
  int nb = 0;
  char buff [80];
  char *type = malloc (80 * sizeof(char));
  char tmp[80];
  int i = 0;
  
  if ( (fd = open ("/etc/mime.types", O_RDONLY)) == -1) {
    perror ("open mime");
    exit (1);
  }


  /* 
     80 + 1 + 74 + 73 + 67 + 72 + 73 + 77 + 65 + 1 + 13 + 1 + 59;; 
  */
  /* virer le commentaires à la con...  */
  for (i = 0; i < 615; i++) {
    read (fd, buff, 1);
  }

  while ( (nb = read (fd, buff, 1) )!= 0) {

  debutboucle:
    /* recherche type  */
    i = 0;
    type[i] = buff[i];
    while ((buff[0] != '\t') && (buff[0] != ' ')
	   && (buff[0] != '\n') ) {
      //      printf ("buff :%c\n", buff[0]);      
      type[i] = buff[0];
      i++;
      read (fd, buff, 1);
    }
    type[i] = '\0';

    /* retirer les tab espaces et retour a la ligne  */
    while ((buff[0] == '\t') || (buff[0] == ' ')
	   || (buff[0] == '\n') || (buff[0] == '#')) {
      
      if ( (buff[0] == '\n') || (buff[0] == '#') )  {

	if (buff[0] == '#') {
	  while (buff[0] != '\n')
	    read (fd, buff, 1);
	}
	read (fd, buff, 1);
	/* a modifier...  */
	goto debutboucle;
      }
      read (fd, buff, 1);
    }
    
lect_ext:
    /* recherche ext  */
    i = 0;
    tmp[i] = buff[i];
    /*tant qu'il y a un vrai car a lire pour une exension */
    while ((buff[0] != '\t') && (buff[0] != ' ')
	   && (buff[0] != '\n') ) {
      tmp[i] = buff[0];
      i++;
      if (read (fd, buff, 1) == 0) {
	close (fd);
	ret = "raté";
	return ;
      }
    }
    tmp[i] = '\0';

#ifdef DEBUG
    //    printf ("ext :%s, vrai :%s\n", tmp, ext);
#endif

    /* si on a trouver la bonne  extension*/
    if (strcmp (tmp, ext) == 0) {
      close (fd);
      /* renvoyer le type mime */
      for (i = 0; i < strlen (type); i++)  {
	ret [i] = type[i];
      }
      return;
    }

    if (buff[0] == ' ') {
      if (read (fd, buff, 1) == 0) {
	close (fd);
	ret = "raté";
	return ;
      }
      goto lect_ext;
    }
    
  }
   

  close (fd);
  ret = "raté";
  return ;
}
