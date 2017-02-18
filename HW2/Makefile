TARGETS = echoserver pop3 smtp
CC = g++
LIBS = -ldl -lstdc++fs

TEST_DIR = test
TEST = echo-test smtp-test pop3-test

TESTS_DIR = tests
TESTS = test_main test_thread test_signal test_filesystem

INC_DIR = include
SRC_DIR = src
OBJ_DIR = obj

CFLAGS = -pthread -std=c++14 -I$(INC_DIR) -Wall -g 

_DEPS = lpi.h server.h string_algorithm.h user.h
DEPS = $(patsubst %,$(INC_DIR)/%,$(_DEPS))

_OBJS = lpi.o server.o string_algorithm.o user.o
OBJS = $(patsubst %,$(OBJ_DIR)/%,$(_OBJS))


all: $(TARGETS)

echoserver: $(OBJS) $(OBJ_DIR)/echoserver.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

smtp: $(OBJS) $(OBJ_DIR)/smtp.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc $(DEPS) _obj
	$(CC) $(CFLAGS) $< -c -o $@

_obj:
	mkdir -p $(OBJ_DIR)


pop3: $(SRC_DIR)/pop3.cc
	g++ $^ -I/opt/local/include/ -L/opt/local/bin/openssl -lcrypto -lpthread -g -o $@

pack:
	rm -f submit-hw2.zip
	zip -r submit-hw2.zip $(SRC_DIR)/*.cc README Makefile $(INC_DIR)/*.hpp $(INC_DIR)/*.h

clean::
	rm -fv $(TARGETS) *~ $(OBJS) *.log $(TESTS) 

realclean:: clean
	rm -fv submit-hw2.zip

tests : $(TESTS)

test_% : tests/test_%.cc
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)
