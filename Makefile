ALL: parent

parent: 
	g++ -c parent_MINE.cpp
	g++ -c child.cpp
	g++ parent_MINE.o child.o

clean:
	rm -f child.o parent_MINE.o a.out
