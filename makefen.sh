#!/bin/bash
for file in `ls ../pgn`; do
   echo ../pgn/file
   ./PGNtoFEN ../pgn/$file ../ecofen/$file
done

cat ../ecofen/*.b.fen | sort > ../ecofen/temp.b.fen
./uniqRR ../ecofen/temp.b.fen > ../ecofen/ecoUniq.b.fen

cat ../ecofen/*.w.fen | sort > ../ecofen/temp.w.fen
./uniqRR ../ecofen/temp.w.fen > ../ecofen/ecoUniq.w.fen

