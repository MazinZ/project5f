CFLAGS = -Wall
CPPFLAGS = -Wall

all: server mcput mclist mcdel mcget #client mcput mcget mcdel mclist

csapp.h:
	wget http://csapp.cs.cmu.edu/2e/ics2/code/include/csapp.h

#csapp.c:
#	wget http://csapp.cs.cmu.edu/2e/ics2/code/src/csapp.c

csapp.o: csapp.h csapp.c

#lib.o: lib.cpp
#	g++ -c lib.cpp

server: server.cpp csapp.o
	g++ $(CPPFLAGS) -g server.cpp csapp.o -lpthread -o server

#client: client.cpp csapp.o
#	g++ $(CPPFLAGS) client.cpp csapp.o -lpthread -o client

#mcput.o:
#	g++ -c mcput.cpp

mcput: mcput.cpp  csapp.o
	g++ $(CPPFLAGS) mcput.cpp csapp.o -lpthread -o mcput

mclist: mclist.cpp csapp.o
	g++ $(CPPFLAGS) -g mclist.cpp csapp.o -lpthread -o mclist

mcdel: mcdel.cpp csapp.o
	g++ $(CPPFLAGS) mcdel.cpp csapp.o -lpthread -o mcdel

mcget: mcget.cpp csapp.o
	g++ $(CPPFLAGS) mcget.cpp csapp.o -lpthread -o mcget

.PHONY: clean
clean:
	/bin/rm -rf csapp.h *.o server mcput mcget mcdel mclist
