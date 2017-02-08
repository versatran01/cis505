TARGETS = smtp pop3 echoserver
OBJS = lpi.o echoserver.o
CC = g++
CFLAGS = -pthread -std=c++14 -I. -Wall -g

all: $(TARGETS)

echoserver: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.cc
	$(CC) $(CFLAGS) $< -c -o $@

smtp: smtp.cc
	g++ $< -lpthread -g -o $@

pop3: pop3.cc
	g++ $^ -I/opt/local/include/ -L/opt/local/bin/openssl -lcrypto -lpthread -g -o $@

pack:
	rm -f submit-hw2.zip
	zip -r submit-hw2.zip *.cc README Makefile

clean::
	rm -fv $(TARGETS) *~ $(OBJS)

realclean:: clean
	rm -fv cis505-hw2.zip
