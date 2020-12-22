/* trouve le jeu no dans le fichier PGN */
/* ./findgame fileName no */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MOTIF1 "[Event"
#define MOTIF2 "1."
#define MAXLEN 10000


bool findName (FILE *fe, int nCible) { /* */
   char line [MAXLEN];
   int n = 1;

   // on passe les n-1 jeux sans imprimer
   while ((n <= nCible - 1) && (fgets (line, MAXLEN, fe) != NULL)) {
      if (strncmp (line, MOTIF2, strlen (MOTIF2)) == 0) n += 1;
   }
   if ((n <= nCible -1) || feof (fe)) {
      printf ("There are: %d games, the target was: %d.\n", n, nCible);
      return false;
   }

   // on recherche MOTIF1 sans imprimer
   while (fgets (line, MAXLEN, fe) != NULL) {
      if (strncmp (line, MOTIF1, strlen (MOTIF1)) == 0) break;
   }
   if (feof (fe)) return false;
   // on imprime a partir de MOTIF 1 jqa MOTIF 2
   printf ("#%d\n%s", n, line);
   while ((fgets (line, MAXLEN, fe) != NULL)) {
      if (strncmp (line, MOTIF2, strlen (MOTIF2)) == 0) break;
      else printf ("%s", line);
   }
   if (feof (fe)) return false;
   // on imprime de MOTIF 2 jqa MOTIF 1
   printf ("%s",line);
   while ((fgets (line, MAXLEN, fe) != NULL)) {
      if (strncmp (line, MOTIF1, strlen (MOTIF1)) == 0) break;
      else printf ("%s", line);
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
