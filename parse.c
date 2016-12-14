/* include libc  */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>


void read_line (int fd, char *ret) {
  char *val = malloc (150 * sizeof (char));
  int i = 0;

#ifdef DEBUG
  printf ("dans le read line\n");
#endif
  if (read (fd, val, 1) == 0) {
    printf ("return");
    ret[0] = '\0';;
    return;
  }
  
  while ( (val[0] != '\n') && (val[0] != '\0') )  {
    ret [i] = val[0];
#ifdef DEBUG
    printf ("lu : %c\n",val[0]);
#endif
    fflush(stdout);
    i++;
    read (fd, val, 1);
  }
#ifdef DEBUG
  printf ("fin lecture, i: %d\n", i);
  fflush(stdout);
#endif

  ret [i]   = '\n';
  ret [i+1] = '\0';

  return;
}


void type_mime (char *ext, char* ret) {
  int fd = 0;
  char line [80] = "a";
  int i = 0, j = 0;
  char *type   =  malloc (150 * sizeof (char));
  char *ext_lu =  malloc (150 * sizeof (char));
  
  if ( (fd = open ("/etc/mime.types", O_RDONLY)) == -1) {
    perror ("open mime");
    exit (1);
  }

  printf ("type mime\n");

  
  /* tant que pas fin du fichier  */
  while (line[0] != '\0') {

  debut:
    
    i = 0;
    j = 0;
#ifdef DEBUG
    printf ("avant read line\n");
    fflush(stdout);
#endif    
    /* lire premiere ligne  */
    read_line (fd, line);
#ifdef DEBUG
    printf ("apres read line\n");
    fflush(stdout);
#endif    


    /* jeter les lignes mal formées */
    if ( (line [0] == '#') || (line [0] == '\n') ) {
#ifdef DEBUG
    printf ("goto2\n");
    fflush(stdout);
#endif    
      goto debut;
    }

#ifdef DEBUG
    printf ("%s", line);
#endif    

    /* stocker la valeur du type (avant espace) */
    while ( (line[i] != '\t') && (line[i] != ' ')  ) {

      /* si pas d'extensions assosiées passer au cas suivant  */
      if (line [i] == '\n') {
#ifdef DEBUG
    printf ("goto1\n");
    fflush(stdout);
#endif    
	goto debut;
      }

      type[i] = line [i];
      i++;
    }
    type [i] = '\0';
    

#ifdef DEBUG
    printf ("type %s\n", type);
#endif    


    /* si pas d'extensions assosiées passer au cas suivant  */
    if (line [i] == '\n') {
#ifdef DEBUG
    printf ("goto\n");
    fflush(stdout);
#endif    
      goto debut;
    }

    
    /* jeter les espaces avant la description */
    while ( (line[i] == '\t') || (line[i] == ' ') ) {
      i++;
    }

#ifdef DEBUG
    printf ("while\n");
    fflush(stdout);
#endif    

    /*  chercher les extensions si il y en a  
	entre dans le while seulement si il y a au moins une ext
    */
    while ( (line[i] != '\n') && (line[i] != '\0') ) {

      /* espace entre les extensions  */
      if (line[i] == ' ') {
	i++;
      }

      j = 0;
      /* tant qu'il y a un char a lire dans l'ext  */
      while ( (line[i] != '\n') && (line[i] != ' ') ) {
	/* rajoute le char lu dans ext_lu  */
#ifdef DEBUG
	printf ("i : %d, j : %d, linei : %c\n", i ,j, line[i]);
	fflush(stdout);
#endif

	/*****    ERREUT SEGMENTATION IICIII *****/
	/*****           POURQUOI?                    *****/
	ext_lu[j] = line [i];
#ifdef DEBUG
	printf ("ext_lu[j] : %c\n", ext_lu[j]);
	fflush(stdout);
#endif
	i++; j++;
      }
      ext_lu [j] = '\0';
      /* comparer l'ext lu avec celle recherchée */
#ifdef DEBUG
      printf ("ext1 %s\n", ext_lu);
      fflush(stdout);
#endif
    
      /* si la même que celle q'uon cherche  */
      if (strcmp (ext_lu, ext) == 0) {
	/* renvoyer le type de l'extension trouvée  */
	for (j = 0; j < strlen (type); j++) {
	  ret[j] = type [j];
	}
#ifdef DEBUG
	printf ("trouvé 1\n");
#endif
	return;
      }

    
    }

    read_line (fd, line);    
  }
  return;
}

  
/*   /\* */
/*      80 + 1 + 74 + 73 + 67 + 72 + 73 + 77 + 65 + 1 + 13 + 1 + 59;; */
/*   *\/ */
/*   /\* virer le commentaires à la con...  *\/ */
/*   for (i = 0; i < 615; i++) { */
/*     read (fd, buff, 1); */
/*   } */

/*   while ( (nb = read (fd, buff, 1) )!= 0) { */

/*   debutboucle: */
/*     /\* recherche type  *\/ */
/*     i = 0; */
/*     type[i] = buff[i]; */
/*     while ((buff[0] != '\t') && (buff[0] != ' ') */
/* 	   && (buff[0] != '\n') ) { */
/*       //      printf ("buff :%c\n", buff[0]); */
/*       type[i] = buff[0]; */
/*       i++; */
/*       read (fd, buff, 1); */
/*     } */
/*     type[i] = '\0'; */

/*     /\* retirer les tab espaces et retour a la ligne  *\/ */
/*     while ((buff[0] == '\t') || (buff[0] == ' ') */
/* 	   || (buff[0] == '\n') || (buff[0] == '#')) { */
      
/*       if ( (buff[0] == '\n') || (buff[0] == '#') )  { */

/* 	if (buff[0] == '#') { */
/* 	  while (buff[0] != '\n') */
/* 	    read (fd, buff, 1); */
/* 	} */
/* 	read (fd, buff, 1); */
/* 	/\* a modifier...  *\/ */
/* 	goto debutboucle; */
/*       } */
/*       read (fd, buff, 1); */
/*     } */
    
/* lect_ext: */
/*     /\* recherche ext  *\/ */
/*     i = 0; */
/*     tmp[i] = buff[i]; */
/*     /\*tant qu'il y a un vrai car a lire pour une exension *\/ */
/*     while ((buff[0] != '\t') && (buff[0] != ' ') */
/* 	   && (buff[0] != '\n') ) { */
/*       tmp[i] = buff[0]; */
/*       i++; */
/*       if (read (fd, buff, 1) == 0) { */
/* 	close (fd); */
/* 	ret = "raté"; */
/* 	return ; */
/*       } */
/*     } */
/*     tmp[i] = '\0'; */

/* #ifdef DEBUG */
/*     //    printf ("ext :%s, vrai :%s\n", tmp, ext); */
/* #endif */

/*     /\* si on a trouver la bonne  extension*\/ */
/*     if (strcmp (tmp, ext) == 0) { */
/*       close (fd); */
/*       /\* renvoyer le type mime *\/ */
/*       for (i = 0; i < strlen (type); i++)  { */
/* 	ret [i] = type[i]; */
/*       } */
/*       return; */
/*     } */

/*     if (buff[0] == ' ') { */
/*       if (read (fd, buff, 1) == 0) { */
/* 	close (fd); */
/* 	ret = "raté"; */
/* 	return ; */
/*       } */
/*       goto lect_ext; */
/*     } */
    
/*   } */
   

/*   close (fd); */
/*   ret = "raté"; */
/*   return ; */
/* } */
