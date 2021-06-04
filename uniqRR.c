/* Elimine les lignes redondantes d'un fichier trie */
/* deux lignes sont considerees equivalentes si le premier champ est egal */
/* ce premier champ est delimite par le separateur SEP */
/* ./uniqRR fileName */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SEP ";"
#define MAXLEN 10000

/*! Lit le fichier fe et envoie sur la sortie standart le fichier resultat
 * \li elimination des lignes equivalentes 
 * \li chaque ligne est au format CSV avec le separateur sep
 * \li deux lignes sont egales si le premier champ est le meme */
bool uniq (FILE *fe, const char *sep) { /* */
   char line1 [MAXLEN];
   char line2 [MAXLEN];
   char lineSave1 [MAXLEN];
   char lineSave2 [MAXLEN];
   char *first1, *first2;

   if (fgets (line1, MAXLEN, fe) == NULL) return false;
   strcpy (lineSave1, line1);
   if ((first1 = strtok (line1, sep)) == NULL) return true;
   
   while (fgets (line2, MAXLEN, fe) != NULL) {
      strcpy (lineSave2, line2);
      if ((first2 = strtok (line2, sep)) == NULL) break;
      if (strcmp (first1, first2) != 0) printf ("%s", lineSave1);
      strcpy (line1, lineSave2);
      strcpy (lineSave1, lineSave2);
      first1 = strtok (line1, sep);
   }
   printf ("%s", lineSave2);
   return true;
}

/*! Programme principal. Lit la commande contenant en parametre le nom du fichier a traiter */
int main (int argc, const char *argv []) {
   FILE *fe;
   if (argc != 2) {
      fprintf (stderr, "Usage: %s <filename>\n", argv [0]);
      exit (EXIT_FAILURE);
   }
   if ((fe = fopen (argv [1], "r")) == NULL) {
      fprintf (stderr, "file: %s not found\n", argv [1]);
      exit (EXIT_FAILURE);
   }
   uniq (fe, SEP);
}
