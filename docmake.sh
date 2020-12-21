#!/bin/bash
declare -a StringArray=("PGNtoFEN.c" "uniqRR.c")
 
for file in ${StringArray[@]}; do
   echo "Fichier $file"
   l=${#file}   
   for ((i = 0; i < $l; i++)); do
      echo -n "-"
   done
   echo "--------"
   grep "\/\*" $file | sed 's/^\([a-zA-Z]\)/\n\1/'
   echo; echo
done
