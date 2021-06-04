/*! \mainpage Traduit un fichier au format PGN en deux fichiers au format FEN
 * \li Synopsys: ./PGNtoFEN [-f] [-p] sourceFile [destFile] 
 * \li -f PGN en Français sinon en anglais
 * \li -p affiche l'echiquer a la console
 * \li si destfile absent sortie standard
 * \li destFile.b.fen pour les noirs (black)
 * \li destFile.w.fen pour les blancs (white)
 * \li ne gere pas en passant */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vt100.h"

#define N 8
#define NTRUNC 1000
#define MAXTURN 8
#define DEPART "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"
#define MAXLIG 10000          // ligne
#define NIL -1

enum {VOID, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, CASTLEKING};
static enum {ENGLISH, FRENCH} lang;
static const char *UNICODE [] = {" ", "♟", "♞", "♝", "♜", "♛", "♚", "♚"};
static const char DICO  [] = {'-', 'P', 'N', 'B', 'R', 'Q', 'K', 'K'};
static const char DICOF [] = {'-', 'P', 'C', 'F', 'T', 'D', 'R', 'R'};
static FILE *fsw, *fsb; 

struct Sdep {
   int piece;
   bool petitRoque;
   bool grandRoque;
   int colDeb;
   int ligDeb;
   int colArr;
   int ligArr;
   int promotion;
   char prise; // - x
   char echec; // + #
};

typedef char tGame_t [N][N];

/*! traduit la piece au format caractere en nombre entier */
static int charToInt (int c, int lang) { /* */
   int signe = islower (c) ? 1 : -1;
   for (unsigned int i = 0; i < sizeof (DICO); i++)
      if (lang == FRENCH) {
         if (toupper (c) == DICOF [i]) return signe * i;
      }
      else {
         if (toupper (c) == DICO [i]) return signe * i;
      }
   return NIL;
}

/*! imprime le jeu a la console pour option -p */
static void printGame (tGame_t jeu, int eval) { /* */
   int v;
   bool normal = true;
   for (int c = 'a'; c <= 'h'; c++) printf (" %c ", c);
   printf ("   --> eval: %d\n", eval);
   for (int l = 7; l >= 0; l--) {
      for (int c = 0; c < N; c++) {
         printf ("%s", (normal ? BG_CYAN : BG_BLACK));
         normal =! normal; 
         v = jeu [l][c];
         printf ("%s %s %s",  (v > 0) ? C_RED : C_WHITE, UNICODE [abs (v)], DEFAULT_COLOR);
      }
      printf ("  %d\n", l+1);
      normal =! normal; 
   }
   printf ("%s\n", NORMAL);
}

/*! le jeu est envoye sous la forme d'une chaine de caracteres au format FEN
  * Forsyth–Edwards Notation */
static void gameToFen (tGame_t jeu, char *sFen, int color) { /* */
   int n, v;
   int i = 0;
   for (int l = N-1; l >=  0; l--) {
      for (int c = 0; c < N; c++) {
         if ((v = jeu [l][c]) != 0) {
            sFen [i++] = (v >= 0) ? tolower (DICO [v]) : DICO [-v];
         }
         else {
            for (n = 0; (c+n < N) && (jeu [l][c+n] == 0); n++);
            sFen [i++] = '0' + n;
            c += n-1;
         }
      }
      sFen [i++] = '/';
   }
   sFen [--i] = ' ';
   sFen [i] = '\0';
   strcat (sFen, (color == 1) ? " b " : " w ");
}

/*! le jeu est recu sous la forme d'une chaine de caracteres
  * fenToGame traduit cette chaine et renvoie l'objet jeu
  * Forsyth–Edwards Notation */
static void fenToGame (const char *sFen, tGame_t jeu) { /* */
   int l = 7, c = 0;
   char car;
   for (unsigned i = 0; i < strlen (sFen) ; i++) {
      car = sFen [i];
      if (isspace (car)) break;
      if (car == '/') continue;
      if (isdigit (car)) {
         for (int k = 0; k < car - '0'; k++) {
            jeu [l][c] = 0;
            c += 1;
         }
      }
      else {
         jeu [l][c] = charToInt (car, ENGLISH);
         c += 1;
      }
      if (c == N) {
         l -= 1;
         c = 0;
      }
   }
}

/*! modifie jeu avec le deplacement dep
 * renvoie faux si le deplacement est manifestement incorrect - controle coherece */
static bool move (tGame_t jeu, struct Sdep dep, int color) { /* */
   int base = (color == -1) ? 0 : 7;
   bool diagDiff = abs (dep.ligArr-dep.ligDeb) != abs (dep.colArr - dep.colDeb); // diag differentes
   bool colLigDiff = (dep.colArr != dep.colDeb && dep.ligArr != dep.ligDeb); // col ou lig differentes
   if (dep.petitRoque) {
      if (abs(jeu [base][4]) != KING || jeu [base][5] != 0 || jeu [base][6] != 0 || abs (jeu[base][7]) != ROOK) {
         fprintf (stderr, "Error: Small castle requested but not possible\n");
         return false;
      }
      jeu [base][4] = 0;
      jeu [base][5] = ROOK * color;
      jeu [base][6] = KING * color;
      jeu [base][7] = 0;
      return true;
   }
   if (dep.grandRoque) {
      if (abs(jeu [base][4]) != KING || jeu [base][3] != 0 || jeu [base][2] != 0 || jeu[base][1] != 0 ||
          abs(jeu [base][0]) != ROOK) {
         fprintf (stderr, "Error: Big castle requested but not possible\n");
         return false;
      }
      jeu [base][4] = 0;
      jeu [base][3] = ROOK * color;
      jeu [base][2] = KING * color;
      jeu [base][0] = 0;
      return true;
   }
   // verif que l'on va sur une case vide ou de la couleur opposée (produit negatif ou nul) 
   if ((jeu [dep.ligDeb][dep.colDeb] * jeu [dep.ligArr][dep.colArr]) > 0) return false;

   // verifications sommaires (non exhaustif) que le deplacement est correct
   switch (abs (dep.piece)) {
   case PAWN:
      if (abs (dep.colArr - dep.colDeb) > 1) return false;
      if (abs (dep.ligArr - dep.ligDeb) > 2) return false;
   break;
   case KNIGHT:
      // if (abs (dep.colArr-dep.colDeb) * abs (dep.ligArr-dep.ligDeb) != 2) return false;
   break;
   case BISHOP:
      if (diagDiff) return false;
   break;
   case ROOK:
      if (colLigDiff) return false;
   break;
   case KING:
      if (abs((dep.colArr-dep.colDeb) * (dep.ligArr-dep.ligDeb)) > 1) return false;
      if (abs (dep.colArr-dep.colDeb) !=1 && (abs (dep.ligArr-dep.ligDeb) != 1)) return false;
   break;
   case QUEEN:
      if (diagDiff && colLigDiff) return false;
   break;
   default:;
   }

   jeu [dep.ligDeb][dep.colDeb]  = 0;
   if ((dep.promotion != 0) && (abs (dep.piece == PAWN))) 
      jeu [dep.ligArr][dep.colArr] = dep.piece * dep.promotion;
   else jeu [dep.ligArr][dep.colArr] = dep.piece;
   return true;
}

/*! Traduit la chaine decrivant un deplacement au format algebrique
 * en structure sDep */
static bool automaton (const char *depAlg, struct Sdep *dep, int color) { /* */
   int etat = 0;
   int sauveEtat = 0;
   int k = 0;
   char car;
   char reste [20];
   strcpy (reste, depAlg);
   dep->colDeb = -1; dep->colArr = -1; dep->ligDeb = -1; dep->ligArr = -1;
   dep->piece = PAWN * color;
   dep->petitRoque = false;
   dep->grandRoque = false;
   dep->promotion = 0;
   dep->prise = '-';
   dep->echec = '-';
   if (strncmp (depAlg, "O-O-O", 5) == 0) {
      dep->grandRoque = true;
      return true;
   }
   if (strncmp (depAlg, "O-O", 3) == 0) {
      dep->petitRoque = true;
      return true;
   }
   // dep->colDeb = 3; dep->colArr = 4; dep->ligDeb = 3; dep->ligArr = 5;

   while ((car = depAlg [k++]) != '\0') {
      switch (car) {
      case ';': sauveEtat = etat; etat = 101; break; // commentaire
      case '\n': if (etat == 101) etat = sauveEtat; break;
      case '{': sauveEtat = etat; etat = 100; break; // commentaire
      case '}': if (etat == 100) etat = sauveEtat; break;
      case '.': case '!': case '?': break;
      case ' ': if (etat >= 6) etat = 9; break;
      case '+': case '#': if (etat >= 6) dep->echec = car; etat = 9; break;
      case '=': if (etat == 6) etat = 7; break; // pour promotion
      case '-': case 'x':
         if (etat <= 3) {
            etat = 4;
            dep->prise = car;
         }
         break;
      case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
         etat = (etat <= 1) ? 2 : 5;
         if (dep->colDeb == -1) dep->colDeb = car - 'a';
         else dep->colArr = car - 'a';
         break;
      case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8':
         etat = (etat <= 1) ? 2 : 5;
         etat = (etat <= 1) ? 3 : 6;
         if (dep->ligDeb == -1) dep->ligDeb = car - '0' - 1;
         else dep->ligArr = car - '0' - 1;
         break;
      default: // R N B Q K R
         if (isupper (car)) {
            if ((etat == 6) || (etat == 7)) {
               etat = 8; 
               dep->promotion = color * abs (charToInt (car, lang));
            } // pas de break : c'est voulu
            if (etat == 0) {
               etat = 1;
               dep->piece = color*(abs(charToInt (car, lang)));
               break; 
            }
         }
      }
   }
   if (dep->colArr == -1) { dep->colArr = dep->colDeb ; dep->colDeb = -1; } // une seule occurence a-h : c'est l'arrivee
   if (dep->ligArr == -1) { dep->ligArr = dep->ligDeb ; dep->ligDeb = -1; } // une seule occurence 1-8 : c'est l'arrivee
   return (etat >= 6);
}

/*! coordonnées de la piece ou des deux pieces identiques dont l'ID est passée en argument
  * renvoie vrai si au moins une piece trouvee */
static  bool find (tGame_t jeu, int piece, int *l1, int *c1, int *l2, int *c2) { /* */
   bool trouve = false;
   for (int l = 0; l < N; l++) {
      for (int c = 0; c < N; c++) {
         if ((jeu [l][c] == piece)) {
            if (! trouve) {
               *l1 = l; *c1 = c;
               trouve = true;
            }
            else {
               *l2 = l; *c2 = c;
               return true;
            }
         }
      }
   }
   return trouve;
}

/*! vrai si toutes les cases de la ligne l entre colonnes cx et cy sont vides */
static bool dumpLine (tGame_t jeu, int l, int cx, int cy) { /* */
   int deb = (cx < cy) ? cx : cy;
   int fin = (cx < cy) ? cy : cx;
   for (int i = (deb + 1); i < fin; i++)
      if (jeu [l][i] != 0) return false;
   return true;
}

/*! vrai si toutes les cases de la colonne c  entre ligne lx et ly sont vide */
static bool dumpColumn (tGame_t jeu, int c, int lx, int ly) { /* */
   int deb = (lx < ly) ? lx : ly;
   int fin = (lx < ly) ? ly : lx;
   for (int i = deb + 1; i < fin; i ++)
      if (jeu [i][c] != 0) return false;
   return true;
}
   
/*!  complete la structure dep en ajoutant l'origine si implicite; Cas du pion */
static bool pawnProcess (tGame_t jeu, struct Sdep *dep) { /* */
   int color = dep->piece; //-1 white, 1: black
   if (dep->prise != 'x') { // pas de prise
      // printf ("colDep %d  ligDep %d colArr %d ligArr %d\n", dep->colDeb, dep->ligDeb, dep->colArr, dep->ligArr);
      if (jeu [dep->ligArr][dep->colArr] != 0) {
         fprintf (stderr, "Error: Pawn move to unvoid case\n"); 
         return false;
      }
      if ((jeu [dep->ligArr + color][dep->colArr]) == dep->piece) {
         dep->colDeb = dep->colArr;
         dep->ligDeb = dep->ligArr + color;
         return true;
      }
      if ((jeu [dep->ligArr + 2 * color][dep->colArr]) == dep->piece) {
         dep->colDeb = dep->colArr;
         dep->ligDeb = dep->ligArr + 2 * color;
         return true;
      }
      fprintf (stderr, "Error: Strange pawn move\n"); 
      return false;
   }
   else { // prise
      if (jeu [dep->ligArr][dep->colArr] * color >= 0) { // le produit devrait etre negatif
         fprintf (stderr, "Error: Pawn should take opposite color\n"); 
         return false;
      }

      if ((dep->ligArr + color < N) && (dep->ligArr + color >= 0) && (dep->colArr-1< N) && (dep->ligArr-1 >= 0) && 
      (jeu [dep->ligArr + color][dep->colArr-1] == dep->piece)) {
         dep->ligDeb = dep->ligArr + color;
         if (dep->colDeb == -1) dep->colDeb = dep->colArr - 1;
      }
      else {
         dep->ligDeb = dep->ligArr + color;
         if (dep->colDeb == -1) dep->colDeb = dep->colArr + 1;
      }
   }
   return true;
}
   
/*! complete la structure dep en ajoutant l'origine definie par
  * dep-> colDeb et dep->colArr si implicite */
static bool complete (tGame_t jeu, struct Sdep *dep) { /* */
   int l1, c1, l2, c2;
   l2 = -1;
   
   if ((dep->colDeb != -1) && (dep->ligDeb != -1)) return true; // deja remplis !
   if (dep->petitRoque || dep->grandRoque) return true; // roque. Rien a completer.
   if (abs (dep->piece) == PAWN) return pawnProcess (jeu, dep); // c'est un pion

   if (! find (jeu, dep->piece, &l1, &c1, &l2, &c2)) {
      fprintf (stderr, "Error: man %d unfound \n", dep->piece);
      return false;
   }
 
   if (l2 == -1) { // une seule piece eligible
      dep->ligDeb = l1;
      dep->colDeb = c1;
      return true;
   }
   
   // cas ou deux pieces identiques pourraient pointer sur la destination
   // ce ne peux être que cavalier ou tour (car reine, roi sont unique, et fou lie a une couleur de case)
   switch (abs(dep->piece)) {
   case BISHOP: // un seul fou peut être sur une meme diagonale
      if (abs (c1 - dep->colArr) == abs (l1 - dep->ligArr)) {
         dep->ligDeb = l1;
         dep->colDeb = c1;
         return true;
      }
      dep->ligDeb = l2;
      dep->colDeb = c2;
      return true;
   case ROOK:
      // printf ("l1 %d c1 %d l2 %d c2 %d\n", l1, c1, l2, c2); 
      // printf ("ligDeb %d coDeb %d\n", dep->ligDeb, dep->colDeb);
      if ((dep->colDeb != -1) && (c1 == dep->colDeb)) { // colonne preremplie
         dep->ligDeb = l1;
         return true;
      } 
      if ((dep->colDeb != -1) && (c2 == dep->colDeb)) { // colonne preremplie
         dep->ligDeb = l2;
         return true;
      } 
      if ((dep->ligDeb != -1) && (l1 == dep->ligDeb)) { // ligne preremplie
         dep->colDeb = c1;
         return true;
      } 
      if ((dep->ligDeb != -1) && (l2 == dep->ligDeb)) { // ligne preremplie
         dep->colDeb = c2;
         return true;
      } 
      if ((c1 == dep->colArr) && (dumpColumn (jeu, c1, l1, dep->ligArr))) {
         dep->ligDeb = l1;
         dep->colDeb = c1;
         return true;
      } 
      if ((c2 == dep->colArr) && (dumpColumn (jeu, c2, l2, dep->ligArr))) {
         dep->ligDeb = l2;
         dep->colDeb = c2;
         return true;
      }
      if ((l1 == dep->ligArr) && (dumpLine (jeu, l1, c1, dep->colArr))) {
         dep->ligDeb = l1;
         dep->colDeb = c1;
         return true;
      }
      if ((l2 == dep->ligArr) && (dumpLine (jeu, l2, c2, dep->colArr))) {
         dep->ligDeb = l2;
         dep->colDeb = c2;
         return true;
      }
      
      // il y a deux pièces eligibles mais pas de depart colonne ou lignes depart identifies
      if ((l2 != -1) && (dep->ligDeb == -1) && (dep->colDeb == -1)) {
         fprintf (stderr, "Error: ambiguous Tower move\n");
         return false;
      }
      break;
   case KNIGHT:
      // les deux cvaliers pointent sur la meme destination
      if ((abs((c1 - dep->colArr) * (l1 - dep->ligArr)) == 2) && abs((c2 - dep->colArr) * (l2 - dep->ligArr)) == 2) {
          if ((dep->colDeb == c1) || (dep->ligDeb == l1)) {
             dep->colDeb = c1;
             dep->ligDeb = l1;
             return true;
          }
          if ((dep->colDeb == c2) || (dep->ligDeb == l2)) {
             dep->colDeb = c2;
             dep->ligDeb = l2;
             return true;
          }
          fprintf (stderr, "Error: Ambiguous move. Two possible kNights\n");
          return false;
      }
      if (abs((c1 - dep->colArr) * (l1 - dep->ligArr)) == 2) { // seule le premier cavalier eligible
         dep->colDeb = c1;
         dep->ligDeb = l1;
         return true;
      }
      if (abs((c2 - dep->colArr) * (l2 - dep->ligArr)) == 2) { // seul le second cavalier eligible
         dep->colDeb = c2;
         dep->ligDeb = l2;
         return true;
      }
      fprintf (stderr, "Error: no kNight can move to specified destination\n");
      return false;
   default: break;
   }
   fprintf (stderr, "Error: unknown piece\n");
   return false;
}

/*! va a la premiere description de deplacement
  * enregistre dans sComment la section commentaires PGN */
static bool syncBegin (FILE *fe, char* sComment) { /* */
   char car1, car2;
   int i = 0;
   while (((car1 = fgetc (fe)) != EOF) && (car1 != '['));
   if (car1 == EOF) return false;
   if ((car1 != '\r') || (car1 != '\n')) sComment [i++] = car1;
   while (true) {
      if (((car2 = fgetc (fe))) == EOF) return false; 
      if (car1 == '\n' && car2 == '1') {
         sComment [i] = '\0';
         sComment [NTRUNC] = '\0';
         return true;
      }
      car1 = car2;
      sComment [i++] = ((car1 == '\r') || (car1 == '\n')) ? ' ' : car1;
   }
   return false;
}

/*! conversion struct en chaine algebrique complete */
static void sprintDep (struct Sdep dep, char *chDep) { /* */
   if (dep.petitRoque) sprintf (chDep, "O-O");
   else if (dep.grandRoque) sprintf (chDep, "O-O-O");
      else sprintf (chDep,"%c%c%d%c%c%d", 
           DICO [abs(dep.piece)], dep.colDeb + 'a', dep.ligDeb+1, dep.prise, dep.colArr + 'a', dep.ligArr+1);
   if (dep.promotion != 0) sprintf (chDep, "%s=%c", chDep, DICO [abs(dep.promotion)]);
   if (dep.echec != '-') sprintf (chDep, "%s%c", chDep, dep.echec);
}

/*! vrai si chDep correspond a la descroption de fin de partie */
static bool isEnd (const char *chDep) { /* */
   if (strncmp (chDep, "1/2-1/2", 7) == 0) return true; // nul
   if (strncmp (chDep, "1-0", 3) == 0) return true; // les blancs gagnent
   if (strncmp (chDep, "0-1", 3) == 0) return true; // les noirs gagnent
   if (strncmp (chDep, "*", 1) == 0) return true; // abandon/
   return false;
}

/*! execute la sequence de deplacement sur jeu */
static bool sequence (tGame_t jeu, char *depAlg, int color, char *sComment) { /* */
   struct Sdep dep;
   char sFEN [MAXLIG];
   char line [MAXLIG];
   if (! automaton (depAlg, &dep, color)) {
      fprintf (stderr, "Error: Automaton in %s\n", depAlg);
      return false;
   }
   if (! complete (jeu, &dep)) return false;
   sprintDep (dep, depAlg);
   gameToFen (jeu, sFEN, color);
   sprintf (line, "%s;%s; %s", sFEN, depAlg, sComment);
   line [NTRUNC] = '\0';
   if (color == -1) fprintf (fsw, "%s\n", line);
   else  fprintf (fsb, "%s\n", line);
   if (move (jeu, dep, color)) return true;
   fprintf (stderr, "Error: Move piece %d \n", dep.piece);
   return false;
}

/*! Programme principal. Lit la line de commande 
 * \li -p : affiche le jeu a la console en format convivial. 
 * \li -f : notation PGN en franais. Sinon english
 * \li le fichier entree est au forlat PGN
 * \li si un nom de fichier dest est fourni, production de deux fichiers fen
 * \li .b.fen pour les noirs .w.fen pour les blancs */
int main (int argc, const char *argv []) { /* */
   char sComment [MAXLIG];
   char game [MAXLIG];
   char fileName [MAXLIG];
   tGame_t jeu;
   int n, indexSource = 1, nTour, nGame = 0;
   char depAlg1 [20];
   char depAlg2 [20];
   bool play = false;
   bool normal;
   FILE *fe; 
   int color = -1; // 1 : black, -1: white
   
   if (argc < 2) { 
      fprintf (stderr, "Usage: [-f] [-p] %s <sourceFile> [destFile]\n", argv [0]);
      exit (EXIT_FAILURE);
   }
   lang = ENGLISH; 
   if (strcmp (argv [1], "-f") == 0) {
      indexSource +=1 ;
      lang = FRENCH;
   }

   if (strcmp (argv [indexSource], "-p") == 0) {
      indexSource += 1;
      play = true;
   }

   if (((fe = fopen(argv [indexSource], "r"))) == NULL){ 
      fprintf (stderr, "File: %s not found\n", argv [indexSource]);
      exit (EXIT_FAILURE);
   }
   if (argc > indexSource + 1) {
      sprintf (fileName, "%s%s", argv [indexSource+1], ".b.fen"); // fichier black
      fsb = fopen (fileName, "w");
      sprintf (fileName, "%s%s", argv [indexSource+1], ".w.fen"); // fichier white
      fsw = fopen (fileName, "w");
   }
   else {
      fsw = stdout; 
      fsb = stdout; 
   }
   
   while (!feof (fe)) {
      fenToGame (DEPART, jeu);
      nGame += 1;
      nTour = 0;
      if (! syncBegin (fe, game)) {
         fprintf (stderr, "End of: %s\n", argv [indexSource]);
         exit (EXIT_SUCCESS);
      }

      while (fscanf (fe, ". %s %s", depAlg1, depAlg2) != 2) {
         if (! syncBegin (fe, game)) {
            exit (EXIT_SUCCESS);
         }
      }
      game [NTRUNC] = '\0';
      printf ("\n%s# %d %s%s\n", C_RED, nGame, game, NORMAL);
      normal = true; 
      do {
         nTour += 1;
         printf ("%d. %s %s\n", nTour, depAlg1, depAlg2);
         sprintf (sComment, "%s/%d/%d %s", argv [indexSource], nGame, nTour, game);
         for (int i = 0; sComment [i]; i++) 
            if (sComment [i] == '"') sComment [i] = '\'';
         if (! sequence (jeu, depAlg1, color, sComment)) {
            normal = false; 
            break;
         }
         if ((strlen (depAlg2) != 0) && !(isEnd (depAlg2))) 
            if (! sequence (jeu, depAlg2, -color, sComment)) {
            normal = false;
            break;
         }
         if (play) printGame (jeu, 0);
      } while (normal && nTour < MAXTURN && fscanf (fe, "%d.%s %s", &n, depAlg1, depAlg2) == 3);
      fprintf (stderr, "%sFile: %s, Game: %d, Change: %d\n", (normal) ? "OK " : "Error: ", argv [indexSource], nGame, nTour);
   }
   fclose (fe);
   fclose (fsw);
   fclose (fsb);
}
