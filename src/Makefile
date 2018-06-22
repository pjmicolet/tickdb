INC=-I../include/
CC=g++
CFLAGS=-O2 -Wall -std=c++14
SOURCES=ticker.cpp print.cpp product.cpp query_engine.cpp query_handler.cpp StringColumn.cpp tickfile.cpp TimeColumn.cpp DoubleColumn.cpp
OBJECTS=$(SOURCES:.cpp=.o)

all: tickdb

tickdb: ticker.o print.o product.o query_handler.o query_engine.o StringColumn.o tickfile.o TimeColumn.o DoubleColumn.o
	$(CC) query_handler.o print.o product.o query_engine.o  StringColumn.o tickfile.o TimeColumn.o DoubleColumn.o ticker.o -o tickdb

ticker.o : ticker.cpp
	$(CC) $(CFLAGS) -c $(INC) ticker.cpp

print.o : print.cpp
	$(CC) $(CFLAGS) -c $(INC) print.cpp

product.o : product.cpp
	$(CC) $(CFLAGS) -c $(INC) product.cpp

query_handler.o : query_handler.cpp
	$(CC) $(CFLAGS) -c $(INC) query_handler.cpp

query_engine.o : query_engine.cpp
	$(CC) $(CFLAGS) -c $(INC) query_engine.cpp

StringColumn.o : StringColumn.cpp
	$(CC) $(CFLAGS) -c $(INC) StringColumn.cpp

tickfile.o : tickfile.cpp
	$(CC) $(CFLAGS) -c $(INC) tickfile.cpp

TimeColumn.o : TimeColumn.cpp
	$(CC) $(CFLAGS) -c $(INC) TimeColumn.cpp

DoubleColumn.o : DoubleColumn.cpp
	$(CC) $(CFLAGS) -c $(INC) DoubleColumn.cpp

clean:
	rm *.o tickdb
