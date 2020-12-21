#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define SEP ";"
#define MAXLEN 10000


bool uniq (const char *fileName, const char *sep) { /* */
   FILE *fe;
   char line1 [MAXLEN];
   char line2 [MAXLEN];
   char lineSave1 [MAXLEN];
   char lineSave2 [MAXLEN];
   char *first1, *first2;

   if ((fe = fopen (fileName, "r")) == NULL) {
      fprintf (stderr, "file: %s not found\n", fileName);
      return false;
   }

   if (fgets (line1, MAXLEN, fe) == NULL) return false;
   strcpy (lineSave1, line1);
   
   if ((first1 = strtok (line1, SEP)) == NULL) {
      // printf ("ici 00 %s\n", line1);
      return true;
   }
   while (fgets (line2, MAXLEN, fe) != NULL) {
      strcpy (lineSave2, line2);
      // printf ("%s...%s\n", lineSave1, line2);
      if ((first2 = strtok (line2, SEP)) == NULL)
         break;
      // printf ("first1: %s, first2: %s, second1: %s, second2: %s\n", first1, first2, second1, second2);
      if (strcmp (first1, first2) != 0) {
         printf ("%s", lineSave1);
      }
      strcpy (line1, lineSave2);
      strcpy (lineSave1, lineSave2);
      first1 = strtok (line1, SEP);
   }
   printf ("%s", lineSave2);
   return true;
}

void main (int argc, char *argv []) {
   if (argc != 2) {
      fprintf (stderr, "Usage: %s <filename>\n", argv [0]);
      exit (0);
   }
   uniq (argv [1], SEP);
}

