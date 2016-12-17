#ifndef PARSE_H
#define PARSE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>


/*  struct for file informations   */
typedef struct {
  char* nom;
  char* extension;
} mr_mime;


/* find the mime type of an file extension  */
/*  parse the mime/type file and store the results in a Mr_mime tab */
mr_mime ** parse_file ( int *count);

/*  search in the tab to find the extension         */
/*  if not in the mime/type file return text/plain  */
int type_mime (mr_mime **tab, char * ext, char *res, int max);

#endif
