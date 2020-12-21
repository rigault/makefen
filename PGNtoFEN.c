/* Tradut un fichier nom au format PGN en deux fichier au format FEN */
/* nom.b.fen pour les noirs (black) */
/* nom.w.fen pour les blancs (white) */
/* ./PGNtoFEN [-f] [-p] %s sourceFile */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#define N 8
#define NTRUNC 1000
#define MAXTURN 10

#define DEPART "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"
#define MAXLIG 10000          // ligne
#define NIL -1
#include "vt100.h"

enum {VOID, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, CASTLEKING};
enum {ENGLISH, FRENCH} lang;
const char *unicode [] = {" ", "♟", "♞", "♝", "♜", "♛", "♚", "♚"};
char dico  [] = {'-', 'P', 'N', 'B', 'R', 'Q', 'K', 'K'};
char dicoF [] = {'-', 'P', 'C', 'F', 'T', 'D', 'R', 'R'};
FILE *fsw, *fsb; 

struct sdep {
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

typedef char TGAME [N][N];

int charToInt (int c, int lang) { /* */
   /* traduit la piece au format caractere... en nombre entier */
   int signe = islower (c) ? 1 : -1;
   for (int i=0; i < sizeof (dico); i++)
      if (lang == FRENCH) {
         if (toupper (c) == dicoF [i]) return signe * i;
      }
      else {
         if (toupper (c) == dico [i]) return signe * i;
      }
   return NIL;
}

void printGame (TGAME jeu, int eval) { /* */
   /* imprime le jeu a la conole pour Debug */
   int l, c;
   int v;
   bool normal = true;
   for (int c = 'a'; c <= 'h'; c++) printf (" %c ", c);
   printf ("   --> eval: %d\n", eval);
   for (l = 7; l >= 0; l--) {
      for (c = 0; c < N; c++) {
         printf ("%s", (normal ? BG_CYAN : BG_BLACK));
         normal =! normal; 
         v = jeu [l][c];
         printf ("%s %s %s",  (v > 0) ? C_RED : C_WHITE, unicode [abs (v)], DEFAULT_COLOR);
      }
      printf ("  %d\n", l+1);
      normal =! normal; 
   }
   printf ("%s\n", NORMAL);
}

void gameToFen (TGAME jeu, char *sFen, int color) { /* */
   /* Forsyth–Edwards Notation */
   /* le jeu est envoye sous la forme d'une chaine de caracteres au format FEN au navigateur */
   /* on revoie les roque White et Black */
   int l, c, n, v;
   int i = 0;
   bool castleW = false;
   bool castleB = false;
   for (l = N-1; l >=  0; l--) {
      for (c = 0; c < N; c++) {
         if ((v = jeu [l][c]) != VOID) {
            if (v == CASTLEKING) castleB = true;
            if (v == -CASTLEKING) castleW = true;
            sFen [i++] = (v >= 0) ? tolower (dico [v]) : dico [-v];
         }
         else {
            for (n = 0; (c+n < N) && (jeu [l][c+n] == VOID); n++);
            sFen [i++] = '0' + n;
            c += n-1;
         }
      }
      sFen [i++] = '/';
   }
   sFen [--i] = ' ';
   sFen [i] = '\0';
   if (color == 1) strcat (sFen, " b ");
   else strcat (sFen, " w ");
   /*if (!castleW) strcat (sFen, "KQ");
   else strcat (sFen, "-");
   if (!castleB) strcat (sFen, "kq");
   else strcat (sFen, "-");*/
}

void fenToGame (char *sFen, TGAME jeu) { /* */
   /* Forsyth–Edwards Notation */
   /* le jeu est recu sous la forme d'une chaine de caracteres du navigateur */
   /* fenToGame traduit cette chaine et renvoie l'objet jeu */
   int i, k, l = 7, c = 0;
   char car;
   for (i = 0; i < strlen (sFen) ; i++) {
      car = sFen [i];
      if (isspace (car)) break;
      if (car == '/') continue;
      if (isdigit (car)) {
         for (k = 0; k < car - '0'; k++) {
            jeu [l][c] = VOID;
            c += 1;
         }
      }
      else {
         jeu [l][c] = charToInt (car, ENGLISH);
//         if (car == 'K' && getInfo.castleW == '1') jeu [l][c] = -CASTLEKING; // le roi blanc a deja roque
//         if (car == 'k' && getInfo.castleB == '1') jeu [l][c] = CASTLEKING; // le roi noir a deja roque
         c += 1;
      }
      if (c == N) {
         l -= 1;
         c = 0;
      }
   }
}

bool move (TGAME jeu, struct sdep dep, int color) { /* */
   /* modifie jeu avec le deplacement dep */
   int base = (color == -1) ? 0 : 7;
   int v;
   if (dep.petitRoque) {
      jeu [base][4] = VOID;
      jeu [base][5] = ROOK * color;
      jeu [base][6] = KING * color;
      jeu [base][7] = VOID;
      return true;
   }
   if (dep.grandRoque) {
      jeu [base][4] = VOID;
      jeu [base][3] = ROOK * color;
      jeu [base][2] = KING * color;
      jeu [base][0] = VOID;
      return true;
   }
   // verifications sommaires que le deplacement est correct
   switch (abs (dep.piece)) {
   case PAWN:
      if (abs (dep.colArr - dep.colDeb) > 1) return false;
      if (abs (dep.ligArr - dep.ligDeb) > 2) return false;
   break;
   case KNIGHT:
      if (abs (dep.colArr-dep.colDeb) * abs (dep.ligArr-dep.ligDeb) != 2) return false;
   break;
   case BISHOP:
      if (abs (dep.ligArr-dep.ligDeb) != abs (dep.colArr - dep.colDeb)) return false;
   break;
   case ROOK:
      if (dep.colArr != dep.colDeb && dep.ligArr != dep.ligDeb) return false;
   break;
   case KING:
      if (abs (dep.colArr-dep.colDeb) !=1 && (abs (dep.ligArr-dep.ligDeb) != 1)) return false;
   break;
   case QUEEN:
      if ((abs (dep.ligArr-dep.ligDeb) != abs (dep.colArr - dep.colDeb)) &&
       (dep.colArr != dep.colDeb) && (dep.ligArr != dep.ligDeb)) return false;
   break;
   default:;
   }
   jeu [dep.ligDeb][dep.colDeb]  = VOID;
   if ((dep.promotion != VOID) && (abs (dep.piece == PAWN))) 
      jeu [dep.ligArr][dep.colArr] = dep.piece * dep.promotion;
   else jeu [dep.ligArr][dep.colArr] = dep.piece;
   return true;
}

bool automaton (char *depAlg, struct sdep *dep, int color) { /* */
   /* traduit la chaine decrivant un deplacement au format algebrique */
   /* en structure sDep decrivant le deplacement */
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
   dep->promotion = VOID;
   dep->prise = '-';
   dep->echec = '-';
   if ((strncmp (depAlg, "O-O-O", 5) == 0) || (strncmp (depAlg, "O-O-O", 5) == 0)) {
      dep->grandRoque = true;
      return true;
   }
   if ((strncmp (depAlg, "O-O", 3) == 0) || (strncmp (depAlg, "O-O", 3) == 0)) {
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
         if (etat <= 3) { etat = 4; dep->prise = car; break; }
      case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
      //printf ("automaton col %c\n", car);
         etat = (etat <= 1) ? 2 : 5;
         if (dep->colDeb == -1) dep->colDeb = car - 'a';
         else dep->colArr = car - 'a';
         break;
      case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8':
      // printf ("automaton lig %c\n", car);
         etat = (etat <= 1) ? 2 : 5;
         etat = (etat <= 1) ? 3 : 6;
         if (dep->ligDeb == -1) dep->ligDeb = car - '0' - 1;
         else dep->ligArr = car - '0' - 1;
         break;
      case 'R': case 'N': case 'Q':
      case 'K': case 'P': case 'B': 
      default: 
         if (isupper (car)) {
            if ((etat == 6) || (etat == 7)) { etat = 8; dep->promotion = color * abs (charToInt (car, lang)); } // pas de break : c'est voulu
            if (etat == 0) { etat = 1; dep->piece = color*(abs(charToInt (car, lang)));  break; }
         }
      }
   }
   if (dep->colArr == -1) { dep->colArr = dep->colDeb ; dep->colDeb = -1; } // une seule occurence a-h : c'est l'arrivee
   if (dep->ligArr == -1) { dep->ligArr = dep->ligDeb ; dep->ligDeb = -1; } // une seule occurence 1-8 : c'est l'arrivee
   return (etat >= 6);
}

bool find (TGAME jeu, int piece, int *l1, int *c1, int *l2, int *c2) { /* */
   /* coordonnées de la piece ou des deux pieces identiques dont l'ID est passée en argument */
   /* renvoie vrai si au moins une piece trouvee */
   int l, c;
   bool trouve = false;
   for (l = 0; l < N; l++) {
      for (c = 0; c < N; c++) {
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

bool dumpLine (TGAME jeu, int l, int cx, int cy) { /* */
   /* vrai si toutes les cases de la ligne l entre colonnes cx et cy sont vides
   */
   int i;
   int deb = (cx < cy) ? cx : cy;
   int fin = (cx < cy) ? cy : cx;
   for (i = (deb + 1); i < fin; i++)
      if (jeu [l][i] != VOID) return false;
   return true;
}

bool dumpColumn (TGAME jeu, int c, int lx, int ly) { /* */
   /* vrai si toutes les cases de la colonne c  entre ligne lx et ly sont vides
   */
   int i;
   int deb = (lx < ly) ? lx : ly;
   int fin = (lx < ly) ? ly : lx;
   for (i = deb + 1; i < fin; i ++)
      if (jeu [i][c] != VOID) return false;
   return true;
}
   
void pawnProcess (TGAME jeu, struct sdep *dep) { /* */
   /*  complete la structure dep en ajoutant l'origine si implicite; Cas du pion
   */
   // printf ("TraiterPion: %d\n", dep->piece);
   int color = dep->piece; //-1 white, 1: black
   if (dep->prise != 'x') {
      if ((jeu [dep->ligArr + color][dep->colArr]) == dep->piece){
         dep->ligDeb = dep->ligArr + color;
      }
      if ((jeu [dep->ligArr + 2 * color][dep->colArr]) == dep->piece){
         dep->ligDeb = dep->ligArr + 2 * color;
      }
      dep->colDeb = dep->colArr;
   }
   else { // prise
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
}
   
void complete (TGAME jeu, struct sdep *dep) { /* */
   /* complete la structure dep en ajoutant l'origine si implicite */
   int l1, c1, l2, c2;
   l2 = -1;
   if ((dep->colDeb != -1) && (dep->ligDeb != -1)) return; // deja remplis !
   if (abs (dep->piece) == PAWN) {
      pawnProcess (jeu, dep);
      return;
   }
   if (! find (jeu, dep->piece, &l1, &c1, &l2, &c2)) {
      fprintf (stderr, "Error: man %d unfound \n", dep->piece);
      exit (0);
   }
 
   if (l2 == -1) { // une seul piece eligible
      dep->ligDeb = l1;
      dep->colDeb = c1;
      // printf ("Une seule piece eligible\n");
      return;
   }
   // printf ("Deux pieces eligibles \n");
   // cas ou deux pieces identiques pourraient pointer sur la destination
   // ce ne peux être que cavalier ou tour ou fou (car reine, roi sont unique)
   switch (abs(dep->piece)) {
   case BISHOP:
      if (abs (c1 - dep->colArr) == abs (l1 - dep->ligArr)) {
         dep->ligDeb = l1;
         dep->colDeb = c1;
         return;
      }
      dep->ligDeb = l2;
      dep->colDeb = c2;
      break;
   case ROOK:
      if ((c1 == dep->colArr) && (dumpColumn (jeu, c1, l1, dep->ligArr))) {
         dep->ligDeb = l1;
         dep->colDeb = c1;
         return;
      } 
      if ((l1 == dep->ligArr) && (dumpLine (jeu, l1, c1, dep->colArr))) {
         dep->ligDeb = l1;
         dep->colDeb = c1;
         return;
      }
      dep->ligDeb = l2;
      dep->colDeb = c2;
      break;
   case KNIGHT:
      // printf ("2 cavaliers de color %d peuvent \n", dep->piece);
      if (abs((c1 - dep->colArr) * (l1 - dep->ligArr)) == 2) {
         if (abs((c2 - dep->colArr) * (l2 - dep->ligArr)) == 2) { // les deux cavaliers pointent su l dest !
            if ((dep->colDeb == c1) || (dep->ligDeb == l1)) {
                dep->colDeb = c1;
                dep->ligDeb = l1;
            }
            else {
               dep->colDeb = c2;
               dep->ligDeb = l2;
            }
         }
         else {
            dep->colDeb = c1;
            dep->ligDeb = l1;
         }
      } 
      else {
         dep->colDeb = c2;
         dep->ligDeb = l2;
      }
      break;
   default: break;
   }
}

bool syncBegin (FILE *fe, char* sComment) { /* */
   /* va a la premiere description de deplacement */
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

void sprintDep (struct sdep dep, char *chDep) { /* */
   /* conversion struct en chaine algebrique complete */
   if (dep.petitRoque) sprintf (chDep, "0-0");
   else if (dep.grandRoque) sprintf (chDep, "0-0-0");
      else sprintf (chDep,"%c%c%d%c%c%d", dico [abs(dep.piece)], dep.colDeb + 'a', dep.ligDeb+1, dep.prise, dep.colArr + 'a', dep.ligArr+1);
   if (dep.promotion != VOID) sprintf (chDep, "%s=%c", chDep, dico [abs(dep.promotion)]);
   if (dep.echec != '-') sprintf (chDep, "%s%c", chDep, dep.echec);
}

bool isEnd (char *chDep) { /* */
   /* vrai si chDep correspon au resultat de fin de partie */
   /* 1/2-1/2 (nul) ; 1-0 (les blancs gagnent) ; 0-1 (les noirs gagnent) ; * (partie interrompue) */
   if (strncmp (chDep, "1/2-1/2", 7) == 0) return true;
   if (strncmp (chDep, "1-0", 3) == 0) return true;
   if (strncmp (chDep, "0-1", 3) == 0) return true;
   if (strncmp (chDep, "*", 1) == 0) return true;
}

bool sequence (TGAME jeu, char *depAlg, int color, char *sComment) { /* */
   struct sdep dep;
   char sFEN [MAXLIG];
   char line [MAXLIG];
   if (! automaton (depAlg, &dep, color)) {
      fprintf (stderr, "Error: automaton in %s\n", depAlg);
      exit (0);
   }
   complete (jeu, &dep);
   sprintDep (dep, depAlg);
   gameToFen (jeu, sFEN, color);
   sprintf (line, "%s;%s; %s;\n", sFEN, depAlg, sComment);
   line [NTRUNC] = '\0';
   if (color == -1) fprintf (fsw, "%s\n", line);
   else  fprintf (fsb, "%s\n", line);
   return move (jeu, dep, color);
}

void test (TGAME jeu, int color) {
   char chDep [MAXLIG];
   struct sdep dep;
   fenToGame (DEPART, jeu);
   strcpy (chDep, "Nf3");
   automaton (chDep, &dep, -1);
   printf ("%d\n", dep.ligArr);
   sprintDep (dep, chDep);
   printf ("chDep auto : %s\n", chDep);
   complete (jeu, &dep);
   sprintDep (dep, chDep);
   printf ("chDep Complet: %s\n", chDep);
   move (jeu, dep, color);
   printGame (jeu, 0);
}

void test2 () {
   TGAME jeu;
   char sFEN [MAXLIG];
   int c1, l1, c2, l2;
   fenToGame (DEPART, jeu);
   gameToFen (jeu, sFEN, 1);
   printf ("%s\n", sFEN);
   if (find (jeu, -ROOK, &l1, &c1, &l2, &c2))
      printf ("trouve %d %d %d %d\n", l1, c1, l2, c2);
   else printf ("non trouve \n");
}

void main (int argc, char *argv []) { 
   char sComment [MAXLIG];
   char game [MAXLIG];
   char fileName [MAXLIG];
   TGAME jeu;
   int n, indexSource = 1, nTour, nGame = 0;
   char depAlg1 [20];
   char depAlg2 [20];
   bool play = false;
   FILE *fe; 
   int color = -1; // 1 : black, -1: white
   
   if (argc < 2) { 
      fprintf (stderr, "Usage: [-f] [-p] %s <sourceFile> [destFile]\n", argv [0]);
      return;
   }
   lang = ENGLISH; 
   if (strcmp (argv [1], "-f") == 0) {
      indexSource +=1;
      printf ("In french\n");
      lang = FRENCH;
   }

   if (strcmp (argv [indexSource], "-p") == 0) {
      indexSource +=1;
      printf ("play\n");
      play = true;
   }

   if (((fe = fopen(argv [indexSource], "r"))) == NULL){ 
      fprintf (stderr, "File: %s not found\n", argv [indexSource]);
      return;
   }
   if (argc > indexSource +1) {
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
         fprintf (stderr, "Error: syncBegin in : %s\n", argv [indexSource]);
         return;
      }

      while (fscanf (fe, ". %s %s", depAlg1, depAlg2) != 2) {
         if (! syncBegin (fe, game)) {
            return;
         }
      }
      game [NTRUNC] = '\0';
      printf ("\n%s# %d %s%s\n", C_RED, nGame, game, NORMAL); 
      do {
         nTour += 1;
         printf ("Tour: %d; %s; %s\n", nTour, depAlg1, depAlg2);
         sprintf (sComment, "%s/%d/%d %s", argv [indexSource], nGame, nTour, game);
         for (int i = 0; sComment [i]; i++) 
            if (sComment [i] == '"') sComment [i] = '\'';
         if (! sequence (jeu, depAlg1, color, sComment)) break;
         if ((strlen (depAlg2) != 0) && !(isEnd (depAlg2))) 
            sequence (jeu, depAlg2, -color, sComment);
         if (play) printGame (jeu, 0);
      } while (nTour < MAXTURN && fscanf (fe, "%d.%s %s", &n, depAlg1, depAlg2) == 3);
      printf ("Nb change: %d\n", nTour);
   }
   fclose (fe);
   fclose (fsw);
   fclose (fsb);
}
