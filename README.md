Fichier PGNtoFEN.c
------------------
/* Traduit un fichier nom au format PGN en deux fichier au format FEN */
/* nom.b.fen pour les noirs (black) */
/* nom.w.fen pour les blancs (white) */
/* ./PGNtoFEN [-f] [-p] sourceFile [destFile] */

int charToInt (int c, int lang) { /* */
   /* traduit la piece au format caractere en nombre entier */

void printGame (TGAME jeu, int eval) { /* */
   /* imprime le jeu a la console pour option -p */

void gameToFen (TGAME jeu, char *sFen, int color) { /* */
   /* Forsyth–Edwards Notation */
   /* le jeu est envoye sous la forme d'une chaine de caracteres au format FEN */

void fenToGame (char *sFen, TGAME jeu) { /* */
   /* Forsyth–Edwards Notation */
   /* le jeu est recu sous la forme d'une chaine de caracteres */
   /* fenToGame traduit cette chaine et renvoie l'objet jeu */

bool move (TGAME jeu, struct sdep dep, int color) { /* */
   /* modifie jeu avec le deplacement dep */
   /* renvoie faux si le deplacement est manifestement incorrect - controle coherece */

bool automaton (char *depAlg, struct sdep *dep, int color) { /* */
   /* traduit la chaine decrivant un deplacement au format algebrique */
   /* en structure sDep */

bool find (TGAME jeu, int piece, int *l1, int *c1, int *l2, int *c2) { /* */
   /* coordonnées de la piece ou des deux pieces identiques dont l'ID est passée en argument */
   /* renvoie vrai si au moins une piece trouvee */

bool dumpLine (TGAME jeu, int l, int cx, int cy) { /* */
   /* vrai si toutes les cases de la ligne l entre colonnes cx et cy sont vides */

bool dumpColumn (TGAME jeu, int c, int lx, int ly) { /* */
   /* vrai si toutes les cases de la colonne c  entre ligne lx et ly sont vide */

void pawnProcess (TGAME jeu, struct sdep *dep) { /* */
   /*  complete la structure dep en ajoutant l'origine si implicite; Cas du pion */

bool complete (TGAME jeu, struct sdep *dep) { /* */
   /* complete la structure dep en ajoutant l'origine si implicite */

bool syncBegin (FILE *fe, char* sComment) { /* */
   /* va a la premiere description de deplacement */
   /* enregistre dans sComment la section commentaires PGN */

void sprintDep (struct sdep dep, char *chDep) { /* */
   /* conversion struct en chaine algebrique complete */

bool isEnd (char *chDep) { /* */
   /* vrai si chDep correspond a la descroption de fin de partie */

bool sequence (TGAME jeu, char *depAlg, int color, char *sComment) { /* */
   /* execute la sequence de deplacement sur jeu */
   /* -p : affiche le jeu a la console en format convivial. Sinon english */
   /* le fichier entree est au forlat PGN */
   /* si un nom de fichier dest est fourni, production de deux fichiers fen */
   /* .b.fen pour les noirs .w.fen pour les blancs */


Fichier uniqRR.c
----------------
/* Elimine les lignes redondantes d'un fichier trie */
/* deux lignes sont considerees equivalentes si le premier champ est egal */
/* ce premier champ est delimite par le separateur SEP */
/* ./uniqRR fileName */

bool uniq (const char *fileName, const char *sep) { /* */


Fichier findgame.c
------------------
/* trouve le jeu no dans le fichier PGN */
/* ./findgame fileName no */

bool findName (FILE *fe, int nCible) { /* */

