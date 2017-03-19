TARGETS = chatserver chatclient udpechoserver
CC = g++

INC_DIR = include
SRC_DIR = src
OBJ_DIR = obj

CFLAGS = -pthread -std=c++14 -I$(INC_DIR) -Wall -g
LFLAGS = -ldl

_DEPS = loguru.hpp
DEPS = $(pathsubst %,$(INC_DIR)/%,$(_DEPS))


.PHONY: all
all: $(TARGETS)

_obj:
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc $(DEPS) _obj
	$(CC) $(CFLAGS) $< -c -o $@ 

chatserver: $(OBJ_DIR)/chatserver.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

chatclient: $(OBJ_DIR)/chatclient.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

udpechoserver: $(OBJ_DIR)/udpechoserver.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

pack:
	rm -f submit-hw3.zip
	zip -r submit-hw3.zip README Makefile *.c* *.h*

clean::
	rm -fv $(TARGETS) *~ *.o submit-hw3.zip

realclean:: clean
	rm -fv cis505-hw3.zip
