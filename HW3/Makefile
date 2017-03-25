TARGETS = chatserver chatclient udpechoserver
CC = g++

INC_DIR = include
SRC_DIR = src
OBJ_DIR = obj

CFLAGS = -pthread -std=c++14 -I$(INC_DIR) -Wall -g
LFLAGS = -ldl -lstdc++fs

_DEPS = loguru.hpp rang.hpp json.hpp string_algorithms.hpp chatutils.hpp servernode.hpp client.hpp address.hpp message.hpp holdbackqueue.hpp
DEPS = $(patsubst %,$(INC_DIR)/%,$(_DEPS))

_OBJS = chatutils.o lpi.o servernode.o client.o string_algorithms.o holdbackqueue.o
OBJS = $(patsubst %,$(OBJ_DIR)/%,$(_OBJS))


.PHONY: all
all: $(TARGETS)

chatserver: $(OBJS) $(OBJ_DIR)/chatserver.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

chatclient: $(OBJS) $(OBJ_DIR)/chatclient.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

udpechoserver: $(OBJ_DIR)/udpechoserver.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc $(DEPS) _obj
	$(CC) $(CFLAGS) $< -c -o $@ 

_obj:
	mkdir -p $(OBJ_DIR)

pack:
	rm -f submit-hw3.zip
	zip -r submit-hw3.zip README Makefile include/* src/*

clean::
	rm -fv $(TARGETS) *~ *.o submit-hw3.zip

realclean:: clean
	rm -fv cis505-hw3.zip
