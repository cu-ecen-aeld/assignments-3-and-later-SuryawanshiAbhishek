#Make file for writer application
#filename:MakeFile
#Author: Abhishek Suryawanshi
#

CROSS_COMPILE =

default: writer

all: writer.o
	$(CROSS_COMPILE)gcc -g -Wall writer.o -o writer

writer.o: writer.c
	$(CROSS_COMPILE)gcc -g -Wall -c -o writer.o writer.c

clean:
	rm -f *.o writer
