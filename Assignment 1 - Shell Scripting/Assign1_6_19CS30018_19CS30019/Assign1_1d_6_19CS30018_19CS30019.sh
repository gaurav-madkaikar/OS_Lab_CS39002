#!/bin/bash

mkdir -p files_mod
for file in *.txt; do
    awk '{ print FNR " " $0 }' "$file" > files_mod/"$file"
    sed -i 's/ /,/g' ./files_mod/"$file"
done