/* trouve le jeu no dans le fichier PGN */
/* ./findgame fileName no */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MOTIF "1."
#define MAXLEN 10000


bool findName (FILE *fe, int nCible) { /* */
   char line [MAXLEN];
   char saveLine [MAXLEN];
   
   int n = 0;

   while ((fgets (line, MAXLEN, fe) != NULL) && (n < nCible)) {
      if (strncmp (line, MOTIF, strlen (MOTIF)) == 0) {
         n += 1;
      }
      strcpy (saveLine, line);
   }
   if ((n < nCible) || feof (fe)) {
      printf ("There are: %d games, the target was: %d.\n", n, nCible);
      return false;
   }
   printf ("#%d\n%s%s", n, saveLine, line);
   while (fgets (line, MAXLEN, fe) != NULL) {
      if (strncmp (line, MOTIF, strlen (MOTIF)) != 0)
         printf ("%s", line);
      else break;
   }
   return true;
}

int main (int argc, char *argv []) {
   FILE *fe;
   if (argc != 3) {
      fprintf (stderr, "Usage: %s <filename> <n>\n", argv [0]);
      exit (EXIT_FAILURE);
   }
   if ((fe = fopen (argv [1], "r")) == NULL) {
      fprintf (stderr, "file: %s not found\n", argv [1]);
      exit (EXIT_FAILURE);
   }
   findName (fe, atoi (argv [2]));
}

