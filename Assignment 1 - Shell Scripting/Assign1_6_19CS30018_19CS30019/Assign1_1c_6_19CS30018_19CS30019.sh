#!/bin/bash

find ./data -type f -wholename '*/*.*' > valid.txt
awk -F. '{print $NF}' valid.txt | sort -u > dirs.txt

mkdir -p ./extensions/Nil/
while read line; do
    mkdir -p ./extensions/"${line}"/
    grep "\.$line\b" valid.txt > matchtmp.txt  
    xargs -a matchtmp.txt mv -t ./extensions/"${line}"/ 
done < dirs.txt

find ./data -type f > invalid.txt
xargs -a invalid.txt mv -t ./extensions/Nil/ 
rm -rf *.txt data/


