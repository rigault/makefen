#bin/bash
letter="ABCDE"
digit="0123456789"

for ((i = 0; i < 5; i++)); do
   for ((j = 0; j <= 9; j++)); do
      for ((k =0 ; k <= 9; k++)); do
         name=${letter:$i:1}$j$k.pgn
         echo $name 
         curl http://www.bookuppro.com/ecopgn/$name > $name
      done
   done
done
