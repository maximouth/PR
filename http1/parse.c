#include "parse.h"

/***************

remettre tout en pointeur de pointeur
tab_ext = pointeur de pointer

*************/


/* create an instance of the struct  */
mr_mime *creer_mr_mime(char* name ,char* ext){
  mr_mime *tmp = (mr_mime *) malloc (sizeof (mr_mime));
  tmp -> nom = strdup (name);
  tmp -> extension = strdup (ext);
  return tmp;
}

/*  read one line of a file and return it  */
int read_line (int fd, char *ret) {
  char val;
  int i = 0;

  if (read (fd, &val, 1) == 0) {
    ret[0] = '\0';
    return 0;
  }

  while ( val!= '\n' && val != '\0' )  {
    ret [i] = val;
    i++;
    read (fd, &val, 1);
  }

  ret [i] = '\0';
  return 1;
}


/*  parse the mime/type file and store the results in a Mr_mime tab */
mr_mime ** parse_file (int *res) {

  int i = 0;

  /* the tab of mr_mime  */
  mr_mime** mr_mim_tab = (mr_mime**) malloc (1500 * sizeof (mr_mime*));
  /* reagexp for the search  */
  char * regexString ="(\\S+)\\s*(.+)*";
  regex_t regexCompiled;

  /* to read the file  */
  char buffer [128];
  int count = 0;

  /* number of match found in the regexp  */
  int match = 0;
  
  size_t maxGroups = 0;
  regmatch_t *groupArray = NULL;
  
  /* try to open mime.types file  */
  int fd=open("/etc/mime.types",O_RDONLY);

  if(fd<0){
    perror("open()");
    exit(errno);
  }

  /* init the mr_mime_tab  */
  for(i = 0; i < 1500; i++) {
    mr_mim_tab[i] = (mr_mime *) malloc (sizeof (mr_mime));
  }

  /* compile the rexexp and store the info in regxcompiled  */
  if (regcomp(&regexCompiled, regexString, REG_EXTENDED))
    {
      perror ("Could not compile regular expression");
      exit (1);
    }

  while(read_line(fd,buffer)) {
    
    if(buffer[0] == '#' || buffer[0]=='\n' || buffer[0]=='\0')
      continue;

    maxGroups = regexCompiled.re_nsub +1;
    groupArray = (regmatch_t*) malloc (sizeof (regmatch_t) * maxGroups);

    match = regexec(&regexCompiled, buffer, maxGroups, groupArray, 0);

    /* si pas d'occurence trouvée  */
    if (match == 0)
      {
	if(maxGroups ==1){
	  continue;
	}

	unsigned int start_0 = groupArray[1].rm_so;
	unsigned int end_0   = groupArray[1].rm_eo;
	unsigned int size_0  = end_0-start_0;
	unsigned int start_1 = groupArray[2].rm_so;
	unsigned int end_1   = groupArray[2].rm_eo;
	unsigned int size_1  = end_1-start_1;

	char* group0 = (char*) malloc( (size_0 +1) * sizeof (char));
	char* group1 = (char*) malloc( (size_1 +1) * sizeof (char));

	strncpy (group0, &buffer[start_0], size_0);
	group0 [size_0] = '\0';
	strncpy (group1, &buffer [start_1], size_1);
	group1[size_1] = '\0';
	const char s[2] = " ";
	if (group1[0] != '\0') {
	  char* tmp=strtok(group1,s);
	  while(tmp != NULL){
	    mr_mim_tab[count++] = creer_mr_mime(group0, tmp);
	    tmp=strtok(NULL, s);
	  }
	}

      }else if ( match == REG_NOMATCH){
      printf ("pas de match\n");
    } else {
      printf ("pas de regexec\n");
    }

  }

  /* return the tab of mr_mime full with ext and mime/type */
  *res = count - 1;
  return mr_mim_tab;
}

/*  search in the tab to find the extension         */
/*  if not in the mime/type file return text/plain  */
int type_mime (mr_mime **tab, char * ext, char *res, int max) {

  int i = 0;
  
  for ( i = 0; i < max + 1; ++i)  {
    if(strcmp(tab[i]->extension, ext) == 0){
	printf("trouver c'est \n %s \n", tab[i]->nom);
	strcpy (res, tab[i]->nom);
	return 0;
      }
    }

  /* not found */
  strcpy (res, "text/plain");
  return 1;


}
