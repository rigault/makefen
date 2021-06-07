/* trouve le jeu no dans le fichier PGN */
/* ./findgame fileName no */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MOTIF "[Event"
#define MAX_LENGTH 10000

/*! lit le fichier fe et cherche le jeu nCible qui est envoye sur la sortie standart */
static bool findName (FILE *fe, int nCible) { /* */
   char lastLine [MAX_LENGTH] = "";
   char line [MAX_LENGTH] = "";
   int n = 1;

   // on recherche MOTIF sans imprimer
   while (fgets (line, MAX_LENGTH, fe) != NULL && n <= nCible) {
      strcpy (lastLine, line);
      if (strncmp (line, MOTIF, strlen (MOTIF)) == 0) n += 1;
   }
   if (feof (fe)) return false;
   // on jqa MOTIF 
   printf ("#%d\n%s", n - 1, lastLine);
   printf ("%s",line);
   while ((fgets (line, MAX_LENGTH, fe) != NULL)) {
      if (strncmp (line, MOTIF, strlen (MOTIF)) == 0) break;
      else printf ("%s", line);
   }
   return true;
}

/*! Programme prinipal. Lit les deux parametres d'entree 
 * \li nom du fichier PGN
 * \li numero du jeu */
int main (int argc, const char *argv []) {
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
   fclose (fe);
}
