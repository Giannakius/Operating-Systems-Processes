ALL: parentdrab

parentdrab: parentdrab.o child.o
	gcc parentdrab.o child.o -o parentdrab -lpthread

parentdrab.o: parentdrab.c
	gcc -g -Wall -c parentdrab.c

child.o: child.c
	gcc -g -Wall -c child.c

clean:
	rm -f parentdrab parentdrab.o child.o
