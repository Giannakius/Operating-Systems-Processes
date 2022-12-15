ALL: parent

parent: 
	g++ -c parent_MINE.cpp
	g++ -c child.cpp
	g++ parent_MINE.o child.o

clean:
	rm -f a.out
