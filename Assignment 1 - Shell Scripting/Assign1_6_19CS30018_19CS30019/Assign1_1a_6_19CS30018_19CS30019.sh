#!/bin/bash

num=$1
if (( $num <= 1 )) 
then
	echo -e "Enter a positive number greater than 1!"
	exit 1 
fi

while (( `expr $num % 2` == 0 )) 
do 
	echo -ne "2 "
	num=$(($num / 2))
done 

i=3
while (( $i <= $num )) 
do
		while (( `expr $num%$i` == 0 )) 
		do 
			echo -ne "$i "
			num=$(($num / $i))	
		done
	i=$(($i + 2))
done

if (( $num > 2 )) 
then 
	echo -ne "$1 " 
fi

echo -ne "\n"