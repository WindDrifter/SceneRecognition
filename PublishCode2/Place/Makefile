CC=g++
CFLAGS=-c -Wall -O3 `pkg-config --cflags opencv`
LDFLAGS=`pkg-config --libs opencv`
SOURCES= util.cpp Leaf.cpp global.cpp main.cpp Sceneloading2.cpp UIUCscene.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=place

all: $(SOURCES) $(EXECUTABLE) CTimage 

$(EXECUTABLE): $(OBJECTS) global.h IntImage.h util.h mdarray.h
	$(CC)  $(OBJECTS) $(LDFLAGS) -o $@

$(EXECUTABLE2): $(OBJECTS2) global.h IntImage.h util.h mdarray.h
	$(CC)  $(OBJECTS2) $(LDFLAGS) -o $@

CTimage: main3.o util.o IntImage.h util.h mdarray.h
	$(CC)  main3.o $(LDFLAGS) util.o -o $@

main3.o: main3.cpp IntImage.h util.h mdarray.h global.h
	$(CC)  main3.cpp -o main3.o $(CFLAGS)
 
util.o: util.cpp util.h mdarray.h
	$(CC) $(CFLAGS) util.cpp -o util.o

Leaf.o: Leaf.cpp global.h IntImage.h util.h mdarray.h
	$(CC) $(CFLAGS) Leaf.cpp -o Leaf.o

global.o: global.cpp global.h IntImage.h util.h mdarray.h
	$(CC) $(CFLAGS) global.cpp -o global.o

main.o: main.cpp global.h IntImage.h util.h mdarray.h
	$(CC) $(CFLAGS) main.cpp -o main.o

UIUCscene.o: UIUCscene.cpp global.h IntImage.h util.h mdarray.h
	$(CC) $(CFLAGS) UIUCscene.cpp -o UIUCscene.o

Sceneloading2.o: Sceneloading2.cpp global.h IntImage.h util.h mdarray.h 
	$(CC) $(CFLAGS) Sceneloading2.cpp -o Sceneloading2.o

clean:
	rm -rf *.o place CTimage *~
