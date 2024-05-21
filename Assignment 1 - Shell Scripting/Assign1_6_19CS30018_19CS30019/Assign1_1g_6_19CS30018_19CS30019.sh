#!/bin/bash

read -rp "Name of the file: " NAME
read -rp "Column number: " COL
read -rp "Regular expression: " REGEX

for i in $(seq 150); do
    for j in $(seq 10); do
        if [ "$j" -eq 1 ]; then echo -ne "${RANDOM}" >> "$NAME"
        else echo -ne ",${RANDOM}" >> "$NAME"
        fi
    done
    echo -ne "\n" >> "$NAME"
done

check=$(awk -F"," "{print \$$COL}" "$NAME" | grep -c "${REGEX}")
if [ "${check}" -gt 0 ]; then echo "True: Match found!"
else echo "False: Match not found!"
fi