TARGETS = echoserver pop3 smtp
OBJS = lpi.o echoserver.o
CC = g++
CFLAGS = -pthread -std=c++14 -I. -Wall -g
TESTS_DIR = tests
TESTS = test_main

all: $(TARGETS)

echoserver: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ -ldl

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
	rm -fv $(TARGETS) *~ $(OBJS) *.log $(TESTS)

realclean:: clean
	rm -fv cis505-hw2.zip

tests : $(TESTS)

test_% : tests/test_%.cc
	$(CC) $(CFLAGS) $< -o $@
