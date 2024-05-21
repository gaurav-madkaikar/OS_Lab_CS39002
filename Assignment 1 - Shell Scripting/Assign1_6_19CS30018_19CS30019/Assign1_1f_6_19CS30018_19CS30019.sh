#!/usr/bin/bash

read -p "Enter input filename: " inp_file
read -p "Enter column number: " col

awk "{print tolower(\$$col)}" $inp_file | sort -r | uniq -c | awk '{print $2,$1}' | sort -k 2 -rno 1e_output_${col}_column.freq
