# Group 6 - OS Lab Assignment 5
# Gaurav Madkaikar - 19CS30018
# Girish Kumar - 19CS30019

run: libmemlab.a
	gcc demo1.c -L. -l memlab -o demo1 -lpthread -lm
	gcc demo2.c -L. -l memlab -o demo2 -lpthread -lm
libmemlab.a: memlab.o
	ar rcs libmemlab.a memlab.o
memlab.o: memlab.c
	gcc -o memlab.o -c memlab.c
clean:
	rm *.a *.o demo1 demo2 demo3 demoE

