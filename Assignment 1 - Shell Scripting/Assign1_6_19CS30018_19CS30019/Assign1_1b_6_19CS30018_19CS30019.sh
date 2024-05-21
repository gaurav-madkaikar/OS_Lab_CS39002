#!/bin/sh

FNAME=$1
if [ $# -eq 0 ]; then
    echo "Enter file name as argument!"
    exit 1
elif [ $# -gt 1 ]; then
    echo "Too many arguments!"
    exit 1
fi

mkdir -p 1.b.files.out/ 
cd ./"$FNAME"/
for file in *.txt; do 
    sort -n "$file" > ../1.b.files.out/"$file"
    cat ../1.b.files.out/"$file" >> ../merge.txt
done 
cd ../ 
cat merge.txt | sort -n | uniq -c > tmp.txt
awk '{print $2 " " $1}' tmp.txt > 1.b.out.txt
rm -rf merge.txt tmp.txt

