TARGETS = echoserver pop3 smtp
CC = g++
LIBS = -ldl -lstdc++fs -lcrypto
LIB = libhw2.so

TEST_DIR = test
TEST = echo-test smtp-test pop3-test

TESTS_DIR = tests
TESTS = test_main test_thread test_signal test_filesystem test_fsm test_regex

INC_DIR = include
SRC_DIR = src
OBJ_DIR = obj
LIB_DIR = lib

CFLAGS = -pthread -std=c++14 -I$(INC_DIR) -I/opt/local/include/ -L/opt/local/bin/openssl -Wall -g   
LDFLAGS = -shared

_DEPS = lpi.h server.h string_algorithms.h user.h smtpserver.h mail.h mailserver.h pop3server.h
DEPS = $(patsubst %,$(INC_DIR)/%,$(_DEPS))

_OBJS_MS1 = lpi.o server.o string_algorithms.o 
OBJS_MS1 = $(patsubst %,$(OBJ_DIR)/%,$(_OBJS_MS1))

_OBJS_MS23 = user.o mail.o mailserver.o 
OBJS_MS23 = $(patsubst %,$(OBJ_DIR)/%,$(_OBJS_MS23))


.PHONY: all
all: $(TARGETS)

echoserver: $(OBJS_MS1) $(OBJ_DIR)/echoserver.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

smtp: $(OBJS_MS1) $(OBJS_MS23) $(OBJ_DIR)/smtp.o $(OBJ_DIR)/smtpserver.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

pop3: $(OBJS_MS1) $(OBJS_MS23) $(OBJ_DIR)/pop3.o $(OBJ_DIR)/pop3server.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc $(DEPS) _obj
	$(CC) $(CFLAGS) $< -c -o $@

_obj:
	mkdir -p $(OBJ_DIR)

pack:
	rm -f submit-hw2.zip
	zip -r submit-hw2.zip $(SRC_DIR)/* README Makefile $(INC_DIR)/*

.PHONY: clean
clean::
	rm -fv $(TARGETS) *~ $(OBJS) *.log $(TESTS) 

realclean:: clean
	rm -fv submit-hw2.zip

tests : $(TESTS)

test_% : tests/test_%.cc
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)

